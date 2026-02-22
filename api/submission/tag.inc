<?php

/**
 * @file
 * API to manage Submission's Tags.
 */

/**
 * Peforms given action for Submission and Tag.
 *
 * @param mixed $accession
 *   Submission accession. E.g., 'TGDR1234'.
 * @param mixed $tag_key
 *   Tag id or name.
 * @param string $action
 *   Action which must be performed.
 *   Currently supported: 'link' and 'unlink'.
 *
 * @return bool
 *   Returns TRUE in case of success and FALSE otherwise.
 */
function tpps_api_submission_tag_action($accession, $tag_key, $action) {
  if (
    empty($accession) || !in_array($action, ['link', 'unlink'])
    || empty($tag_id = SubmissionTag::getId($tag_key))
    || empty($tag = SubmissionTag::load($tag_id))
    || $tag['static']
   ) {
    return FALSE;
  }
  if ($action == 'link') {
    SubmissionTag::link($tag_id, $accession);
  }
  elseif ($action == 'unlink') {
    SubmissionTag::unlink($tag_id, $accession);
  }
  return TRUE;
}
