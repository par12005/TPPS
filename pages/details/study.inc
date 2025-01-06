<?php

/**
 * @file
 * TPPS Study details page.
 *
 * /tpps/details/<accession>
 */

/**
 * Starting point for most TPPS Details pages.
 *
 * Either returns the top details page, which allows users to select from
 * completed studies, or returns the per-submission details page, which allows
 * users to browse the details of a study.
 *
 * @param string $accession
 *   The accession to get details from. If NULL, show all submissions.
 *
 * @return string
 *   The appropriate output.
 */
function tpps_details_main($accession = NULL) {
  global $base_url;
  global $user;

  // @TODO This should never happen because Drupal Menu system won't allow
  // empty 2nd path element. Check /tpps/details/' case and remove if possible.
  if (empty($accession)) {
    drupal_goto('tpps/details');
  }

  $output = l(t('Return to TPPS Details'), $base_url . '/tpps/details');
  $submission = new Submission($accession);
  if ($submission->doesExist()) {
    if (isset($submission->state['study_view_role'])) {
      // If more than 0, then a view role was set by curation team
      if ($submission->state['study_view_role'] > 0) {
        if (
          !in_array(user_roles($true)[$submission->state['study_view_role']], $user->roles)
          // @TODO Minor. Admins must always has access so this check could
          // be done earlier and avoid extra checks.
          && !in_array('administrator', $user->roles)
        ) {
          $output .= t("<br />This study has been set to be available to only "
            . "a specific user role. <br />If you believe this is an error, "
            . "please contact us via the contact us page.<br />");
          // @TODO Minor. Use drupal_set_message() and redirect to 'tpps/detals' page.
          return $output;
        }
      }
    }

    if ($submission->state['status'] != TPPS_SUBMISSION_STATUS_APPROVED) {
      drupal_set_message(
        t('This submission is still under construction. Some of the information '
        . 'may be available here, but please keep in mind that it may be '
        . 'incomplete or subject to change in the near future.'),
        'warning'
      );
      if (
        variable_get('tpps_details_use_external_tabs', FALSE)
        && function_exists('tpps_details_byaccession_page')
      ) {
        $output .= @tpps_details_byaccession_page()['raw_markup']['#markup'];
        tpps_add_css_js('main');
        return $output;
      }
    }

    // Note: There is no file id because used processed data from all the files
    // for all the species.
    $tree_info = [];
    if (!empty($submission->sharedState['tree_info'])) {
      foreach ($submission->sharedState['tree_info'] as $name => $info) {
        if (!empty($info['lat']) && !empty($info['lng'])) {
          $tree_info[] = [$name, $info['lat'], $info['lng']];
        }
      }
      drupal_add_js(['tpps' => ['tree_info' => $tree_info]], 'setting');
    }
    // @TODO Add JS using drupal_add_js().
    $output .= tpps_get_markercluster_code();
    tpps_add_css_js('main');

    if (!empty($pub_link = ($submission->state['title'] ?? NULL))) {
      $pub_id = db_select('chado.project_pub', 'p')
        ->fields('p', array('pub_id'))
        ->condition('project_id', $submission->state['ids']['project_id'])
        ->execute()->fetchObject()->pub_id ?? NULL;
      if (!empty($pub_id)) {
        $bundle = tripal_load_bundle_entity(array('label' => 'Publication'));
        $pub_id = chado_get_record_entity_by_bundle($bundle, $pub_id);
        $pub_link = "<a href='$base_url/Publication/$pub_id'>$pub_link</a>";
      }
      $output .= "<h2>$pub_link</h2>";
    }
    $output .= tpps_show_tags(tpps_submission_get_tags($accession));
    if ($primary_author = $submission->state['authors'][0] ?? NULL) {
      if (preg_match('/,/', $primary_author)) {
        $parts = explode(',', $primary_author);
        $lastname = trim($parts[0]);
      }
      else {
        $parts = explode(' ', $primary_author);
        $lastname = $parts[count($parts) - 1];
      }
      $output .= "<h4>$lastname et al {$submission->state['pyear']}";
      if (!empty($submission->state['journal'])) {
        $output .= " published in {$submission->state['journal']}";
      }
      $output .= "</h4>";
      // @TODO Minor. Use l() and drupal_add_js().
      $output .= "<div id=\"abstract\"><a href=\"#abstract\">Click to show abstract</a></div>"
        . "<script>jQuery(document).ready(jQuery('#abstract').on('click', function() {
            jQuery('#abstract')[0].innerHTML = `{$submission->state['abstract']}`;
          }));</script>";
    }
    else {
      $output .= '<br />';
    }
    if (module_exists('cartogratree')) {
      $items[] = l(t('View in CartograPlant'), 'cartogratree',
        [
          'query' => ['accession' => $accession],
          'attributes' => ['target' => 'blank'],
        ]
      );
      $items[] = l(t('Download compressed raw files'),
        'tpps/api/files/' . $accession, ['attributes' => ['target' => 'blank']]
      );
    }

    $page1_values = &$submission->state['saved_values'][TPPS_PAGE_1];
    if (
      $page1_values['publication']['status'] != 'Published'
      && $submission->isCreator()
    ) {
      $items[] = l(t('Edit publication information'), 'tpps/' . $accession . '/edit-publication');
    }
    $output .= theme('item_list', ['items' => $items]);
    $output .= tpps_details_tab_list($submission);
    return $output;
  }

  drupal_set_message(t('This submission is pending resubmission through the '
    . 'new TPPS pipeline. Some of the information may be available here, '
    . 'but please keep in mind that it may be incomplete or subject to '
    . 'change in the near future.'),
    'warning'
  );

  if (function_exists('tpps_details_byaccession_page')) {
    $output .= @tpps_details_byaccession_page()['raw_markup']['#markup'];
    // [VS].
    tpps_add_css_js('main');
    // [/VS].
  }
  return $output;
}

/**
 * Gets rendered list of tabs for study details page.
 *
 * To add new tab - just add new element to $tab_list array.
 * Note that all links in content will be replaced with current path and
 * fragment so update /js/tpps.js if you don't want that. Search 'submission'.
 *
 * @param array $submission
 *   Study Submission object.
 *
 * @return string
 *   HTML for the details tabs.
 */
function tpps_details_tab_list($submission) {
  $state = &$submission->state;
  $page4 = $state['saved_values'][TPPS_PAGE_4] ?? NULL;

  $tab_items = [];
  $content = [];
  // Required tabs.
  $tabs_list = [
    'species' => 'Species',
    'study' => 'Study Details',
    'trees' => 'Plants',
  ];
  // Conditional tabs.
  if ($submission->hasPhenotypeData()) {
    $tabs_list['phenotype'] = 'Phenotypes / Environments';
  }
  if ($submission->hasGenotypeData()) {
    $tabs_list['genotype'] = 'Genotypes';
  }
  if ($submission->hasMassSpectrometryData()) {
    $tabs_list['mass_spectrometry'] = 'Mass Spectrometry';
  }
  // Build HTML.
  // Only first element must have class 'active'.
  $content_class = 'tab-pane fade in active';
  foreach ($tabs_list as $key => $title) {
    $tab_items[] = [
      'data' => l(t($title), '',
      [
        'external' => TRUE,
        'fragment' => $key,
        'attributes' => [
          'class' => ['nav-link'],
          'role' => 'tab',
          'data-toggle' => 'tab',
        ],
      ]),
      'class' => ['nav-item'],
    ];
    $content[] = '<div id="' . $key . '" role="tabpanel" '
      . 'class="' . $content_class . '"></div>';
    $content_class = 'tab-pane fade';
  }
  return theme('item_list', [
    'items' => $tab_items,
    'type' => 'ul',
    'attributes' => [
      'class' => ['nav', 'nav-tabs'],
      'role' => 'tablist',
    ],
  ]) . '<div id="tab-content" class="tab-content">' . implode('', $content) . '</div>';
}
