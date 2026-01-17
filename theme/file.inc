<?php

/**
 * @file
 * Files related theme-functions.
 */

/**
 * Wraps file id into hyperlink.
 *
 * @param array $variables
 *   Data for theming.
 *   'fid' - string. File Id.
 *
 * @return string
 *   Returns string where 1st char is wrapped with '<strong>' HTML-tag.
 */
function theme_file_id(array $variables) {
  $fid = check_plain($variables['fid']) ?? NULL;
  if (empty($fid)) {
    return '';
  }
  $options = [
    'query' => ['token' => variable_get('tpps_api_token')],
    'attributes' => $variables['attributes'] ?? [],
  ];
  if (!empty($variables['destination'])) {
    $options['query']['destination'] = $variables['destination'];
  }
  return l($fid, 'api/file/' . $fid . '/info', $options);
}

/**
 * Renders local VCF-file path as link to API page.
 *
 * @param array $variables
 *   Data for theming.
 *   'path' - string. Path to file
 *   'accession' - string. Submission accession. E.g., 'TGDR1234'.
 *
 * @return string
 *   Returns link to API page where submission's VCF files are listed.
 */
function theme_file_local_vcf(array $variables) {
  $path = check_plain($variables['path']) ?? NULL;
  if (empty($path)) {
    return '';
  }
  $accession = check_plain($variables['accession']) ?? NULL;
  $options = [
    'attributes' => $variables['attributes'] ?? [],
  ];
  if (!empty($variables['destination'])) {
    $options['query']['destination'] = $variables['destination'];
  }
  return l($path, 'api/submission/' . $accession . '/vcf', $options);
}

/**
 * Wraps file id into hyperlink.
 *
 * @param array $variables
 *   Data for theming.
 *   'fid' - string. File Id.
 *   'attributes' - array. List of hyperlink attributes.
 *   'destination' - string. Optional Drupal's 'destination' link attribute.
 *
 * @return string
 *   Returns string where 1st char is wrapped with '<strong>' HTML-tag.
 */
function theme_file_action_list(array $variables) {
  if (
    empty($accession = check_plain($variables['accession']))
    || empty($fid = check_plain($variables['fid']))
  ) {
    return '';
  }
  $options = [
    'query' => [
      'token' => variable_get('tpps_api_token'),
    ],
    'attributes' => $variables['attributes'] ?? [],
  ];
  if (!empty($variables['destination'])) {
    $options['query']['destination'] = $variables['destination'];
  }
  return '[' . implode('|', [
    l(t('D'), 'api/file/' . $fid . '/get',
      array_merge($options, ['attributes' => ['title' => t('Download File')]])
    ),
    TppsFile::buildApiUsageLink($fid, 'U',
      array_merge($options, ['attributes' => ['title' => t('Usage')]])
    ),
    l(t('U+'), 'api/file/' . $fid . '/usage/add/' . $accession,
      array_merge($options, ['attributes' => ['title' => t('Add 1 Usage')]])
    ),
    l(t('U-'), 'api/file/' . $fid . '/usage/delete/' . $accession,
      array_merge($options, ['attributes' => ['title' => t('Delete 1 Usage')]])
    ),
  ]) . ']';
}
