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
  $destination = $variables['destination'] ?? NULL;
  if (empty($fid)) {
    return '';
  }
  $options = [
    'query' => [
      'token' => variable_get('tpps_api_token'),
      'destination' => $destination,
    ],
    'attributes' => $variables['attributes'] ?? '',
  ];
  return l($fid, 'api/file/' . $fid . '/info', $options);
}

/**
 * Wraps file id into hyperlink.
 *
 * @param array $variables
 *   Data for theming.
 *   'fid' - string. File Id.
 *   'attributes' - array. List of hyperlink attributes.
 *
 * @return string
 *   Returns string where 1st char is wrapped with '<strong>' HTML-tag.
 */
function theme_file_action_list(array $variables) {
  $accession = $variables['accession'] ?? NULL;
  $destination = $variables['destination'] ?? NULL;
  if (empty($accession)) {
    return '';
  }
  $fid = check_plain($variables['fid']) ?? NULL;
  if (empty($fid)) {
    return '';
  }
  $options = [
    'query' => [
      'token' => variable_get('tpps_api_token'),
      'destination' => $destination,
    ],
    'attributes' => $variables['attributes'] ?? '',
  ];
  return '[' . implode('|', [
    l(t('D'), 'api/file/' . $fid . '/get',
      array_merge($options, ['attributes' => ['title' => t('Download File')]])
    ),
    l(t('U'), 'api/file/' . $fid . '/usage',
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
