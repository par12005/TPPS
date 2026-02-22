<?php

/**
 * @file
 * Contans theme-functions related to TPPS Submissions.
 */

/**
 * Themes list of submission's tags.
 *
 * Displays tags with proper styling.
 * Used at Study Details pages and tabs.
 *
 * Required keys:
 *   'tags' - array
 *     Assocciative array where key is tag id and value is array with keys
 *     which are the represent fields of the DB table 'tpps_tag'.
 */
function theme_submission_tag_list(array $variables) {
  if (!empty($variables['tags'])) {
    foreach ($variables['tags'] as $tag) {
      if (empty($tag)) {
        continue;
      }
      $color = check_plain($tag['color']);
      $display = ($display ?? '') . '<span class="tag" style="background-color:'
        . (empty($color) ? 'white' : $color) . '"><span class="tag-text">'
        . check_plain($tag['name']) . '</span></span>';
    }
    tpps_add_css_js('main');
    return $display ?? '';
  }
}

/**
 * Themes dates for TPPS Admin Panel.
 */
function theme_submission_date(array $variables) {
  $date = check_plain($variables['date']);
  return !empty($date) ? date("F j, Y, g:i a", $date) : t('Unknown');
}

/**
 * Themes submitter of the TPPS Submission.
 */
function theme_submission_submitter($variables) {

  $use_tripal_contact = FALSE;
  if ($use_tripal_contact) {
    // @TODO Review this code. Copied from TPPS Admin Panel when it was updated
    // to use 'SubmissionCache' class. Not tested.
    $contact_bundle = tripal_load_bundle_entity(
      ['label' => 'Tripal Contact Profile']
    );
    // If Tripal Contact Profile is available, we want to link to the
    // profile of the owner instead of just displaying the name.
    if ($contact_bundle) {
      $query = new EntityFieldQuery();
      $results = $query->entityCondition('entity_type', 'TripalEntity')
        ->entityCondition('bundle', $contact_bundle->name)
        ->fieldCondition('local__email', 'value', $owner_mail)
        ->range(0, 1)
        ->execute();
      // [VS]
      if (!empty($results['TripalEntity'])) {
        // Commented out because there is a lot of warning messages
        // shown at page.
        //
        //  $entity = current(
        //    array_reverse(
        //      entity_load(
        //        'TripalEntity',
        //        array_keys($results['TripalEntity'])
        //      )
        //    )
        //  );
        //  $owner = "<a href=\"$base_url/TripalContactProfile/"
        //    . "{$entity->id}\">$submitting_user</a>";
      }
      // [/VS]
    }
  }
  $submitter = $variables['submitter'] ?? NULL;
  if (!empty($submitter['name'])) {
    return $submitter['name']
    . ($submitter['mail'] ? ' (' . $submitter['mail'] . ')' : '');
  }
  if (!empty($submitter['mail'])) {
    return $submitter['mail'];
  }
  return $submitter['uid'] ?? '';
}

/**
 * Highlights number in study accession.
 *
 * Usage: theme('submission_accession_link', $vars);
 *
 * @param array $variables
 *   Data for theming.
 *   'accession' - String to be processed.
 *
 * @return string
 *   Returns string where 1st char is wrapped with '<strong>' HTML-tag.
 */
function theme_submission_accession_link(array $variables) {
  $accession = $variables['accession'] ?? NULL;
  $path = $variables['path'] ?? NULL;
  $show_version = $variables['show_version'] ?? FALSE;
  // Loaded submission means it was processed and data was loaded into database.
  $show_load_time = $variables['show_load_time'] ?? FALSE;

  if (empty($accession)) {
    return '';
  }
  $number = str_replace('TGDR', '', ($accession));
  $output = '<span class="tpps-study-accession">TGDR</span>'
    . '<span class="tpps-study-accession-number" title="'
    . t('Study accession number') . '">' . $number . '</span>';
  if ($show_version || $show_load_time) {
    $cached_submission = SubmissionCache::get($accession);
  }

  // Extra data.
  if ($show_version) {
    $submission_version = $cached_submission[SubmissionVersion::FORM_KEY] ?? NULL;
    if (!empty($submission_version)) {
      $output .= ' {<span class="tpps-study-form-version-number" title="'
        .  t('Submission Form Version Number')
        . '">' . $submission_version . '</span>}';
    }
  }
  if ($show_load_time) {
    // @TODO Minor. Create separate theme-function.
    $load_time = $cached_submission['date']['loaded'] ?? NULL;
    $project_id = $cached_submission['project_id'] ?? NULL;
    if (!empty($project_id) && !empty($load_time)) {
      $output .= ' [Loaded]';
      // format_date($load_time,  'custom', "Y/m/d H:i:s O");
    }
  }
  if ($path) {
    $output = l($output, $path,
      [
        'html' => TRUE,
        'attributes' => ['class' => ['submission_accession_link']],
      ]
    );
  }
  return $output;
}

/**
 * Shows submission's actions.
 *
 * Uses static caching for action list template.
 * Usage: theme('submission_actions', $vars);
 *
 * @param array $variables
 *   Data for theming.
 *   'accession' - string. Submission's acession. E.g., 'TGDR1234'.
 *   'deposit_id' - Integer. Zenodo Deposit Id.
 *
 * @return string
 *   Returns string where 1st char is wrapped with '<strong>' HTML-tag.
 */
function theme_submission_actions(array $variables) {
  $accession = $variables['accession'] ?? NULL;
  $deposit_id = $variables['deposit_id'] ?? NULL;

  $token = drupal_encode_path('[accession]');
  $static_cache = &drupal_static(__FUNCTION__);
  if (empty($static_cache)) {
    $action_list = [
      implode(' | ', [
        l(t('Edit'), 'tppsc/[accession]'),
        l(t('Details'), 'tpps/details/[accession]'),
        l(t('Files'), 'tpps-admin-panel/file-diagnostics/[accession]'),
      ]),
      'JSON: ' . implode(' | ', [
        l(t('State'), 'api/submission/[accession]/view/state'),
        l(t('Shared State'), 'api/submission/[accession]/view/shared_state'),
      ]),
      implode(' | ', [
        l(t('View'), 'tpps/submission/[accession]/view'),
        l(t('Export'), 'tpps/submission/[accession]/export'),
        l(t('Clone'), 'tpps/submission/[accession]/clone')
      ]),
      'Zenodo: ' . implode(' | ', [
        l(t('Deposit'), 'tpps/submission/[accession]/zenodo').
        l(t('Files'), 'tpps/submission/[accession]/zenodo/files'),
      ]),
      'Dataset DOI: '
        . l(t('Create'),
          'tpps/[accession]/dataset-doi/create',
          ['attributes' => ['class' => $token . '_create_link']]
        )
        . l(t('View'),
          'tpps/[accession]/dataset-doi',
          ['attributes' => ['class' => $token . '_view_link']]
        ),
      l(t('Rename local VCF'), 'tpps/submission/[accession]/vcf/local/rename'),
      l(
        t('Edit publication information'),
        'tpps/[accession]/edit-publication',
        ['attributes' => ['class' => $token . '_edit_publication_information_link']]
      ),
    ];
    // To add items use $action_list and recreate element.
    $template = theme('item_list', [
      'items' => $action_list,
      'attributes' => ['class' => ['admin-panel-action-list']],
    ]);
    $static_cache = $template;
  }
  $template = $static_cache;
  $output = str_replace($token, $accession, $template);

  if ($variables['edit_publication_information'] === TRUE) {
    $output .= '<style>.admin-panel-action-list li:has(.' . $accession
      . '_edit_publication_information_link) {display: none;}</style>';
  }

  $name = empty($deposit_id) ? 'create' : 'view';
  $output .= '<style>.admin-panel-action-list .' . $accession . '_'
    . $name . '_link {display: none;}</style>';
  return $output;
}
