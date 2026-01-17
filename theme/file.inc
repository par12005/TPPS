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
 * Shows file size.
 *
 * @param array $variables
 *   Data for theming.
 *   'size' - int. Size of the file in bytes.
 *
 * @return string
 *   Returns formatted HTML string.
 */
function theme_file_size(array $variables) {
  $size = check_plain($variables['size']) ?? NULL;
  return ($size ? '(' . format_size($size) . ') ' : '');
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
 *   'tiny' - bool. Optional. Then true short labels for links will be used.
 *      Otherwise long strings will be used.
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
  // Use 'tiny' format by default.
  // Set to FALSE explicitly to have 'full' format.
  $is_tiny = !isset($variables['tiny']) || $variables['tiny'];
  $link_list = [
    'download' => [
      'label' => t('D'),
      'title' => t('Download File'),
      'path' => 'api/file/' . $fid . '/get',
    ],
    'usage' => [
      'label' => t('U'),
      'title' => t('Usage'),
      'path' => 'api/file/' . $fid . '/usage',
    ],
    'add_usage' => [
      'label' => t('U+'),
      'title' => t('Increase Usage'),
      'path' => 'api/file/' . $fid . '/usage/add/' . $accession,
    ],
    'delete_usage' => [
      'label' => t('U-'),
      'title' => t('Decrease Usage'),
      'path' => 'api/file/' . $fid . '/usage/delete/' . $accession,
    ],
  ];
  $output = [];
  foreach ($link_list as $key => $link) {
    $options = array_merge(
      $options,
      ['attributes' => ['title' => $link['title']]]
    );
    $label = $link['label'];
    $space = '';
    if (!$is_tiny) {
      // Long.
      $label = str_replace(' ', '&nbsp;', $link['title']);
      $options['html'] = empty($variables['tiny']);
      $space = '&nbsp;';
    }
    $output[] = l($label, $link['path'], $options);
  }

  if (empty($output)) {
    return '';
  }
  return '[' . $space . implode($space . '|' . $space, $output) . $space . ']';
}
