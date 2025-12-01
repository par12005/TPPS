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
 */
function theme_submission_tag_list(array $variables) {
  if (!empty($variables['tags'])) {
    foreach ($variables['tags'] as $tag_id) {
      $tag = SubmissionTag::load($tag_id);
      $display = ($display ?? '') . '<span class="tag" style="background-color:'
        . (empty($tag->color) ? 'white' : $tag->color)
        . '"><span class="tag-text">' . $tag->name . '</span></span>';
    }
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

  if (empty($accession)) {
    return;
  }
  $data = SubmissionCache::getAll();
  $number = str_replace('TGDR', '', ($accession));
  $output = '<span class="tpps-study-accession">TGDR</span>'
    . '<span class="tpps-study-accession-number" title="'
    . t('Study accession number') . '">' . $number . '</span>';

  // Extra data.
  if ($show_version) {
    $submission_version = $data[$accession][SubmissionVersion::FORM_KEY] ?? NULL;
    if (!empty($submission_version)) {
      $output .= ' {<span class="tpps-study-form-version-number" title="'
        .  t('Submission Form Version Number')
        . '">' . $submission_version . '</span>}';
    }
  }
  if ($path) {
    $output = l($output, $path, ['html' => TRUE]);
  }
  return $output;

}
