<?php

/**
 * @file
 * API Callback. Lists file usage.
 */

/**
 * API Callback. Shows file usage.
 *
 * @param object $file
 *   Fully loaded managed file object.
 */
function tpps_api_file_usage_list($file) {
  return file_usage_list($file);
}
