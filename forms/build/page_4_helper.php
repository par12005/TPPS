<?php
/**
 * @file
 * Define the helper functions for the GxPxE Data page.
 */

/**
 * Generates disabled managed field.
 *
 * When file already was uploaded.
 *
 * @param array $fields
 *   Drupal Form API array with fields.
 * @param string $file_field_name
 *   Name of the managed file field.
 *
 *   @TODO Check if it's in use and remove if possible.
 */
function tpps_build_disabled_file_field(array &$fields, $file_field_name) {
  $fields['files'][$file_field_name] = [
    '#type' => 'managed_file',
    '#tree' => TRUE,
    '#access' => FALSE,
  ];
}
