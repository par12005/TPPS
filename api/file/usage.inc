<?php

/**
 * @file
 * API Callback. Return's file usage info in JSON format.
 */

/**
 * API Callback. Return's file usage info in JSON format.
 *
 * @param object $file
 *   Fully loaded managed file object.
 */
function tpps_api_file_usage($file) {
  return file_usage_list($file);
}
