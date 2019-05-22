<?php

/**
 * @file
 */

/**
 *
 */
function ajax_date_year_callback(&$form, $form_state) {

  return $form['EndingDate']['year'];
}

/**
 *
 */
function ajax_date_month_callback(&$form, $form_state) {

  return $form['EndingDate']['month'];
}

/**
 *
 */
function page_2_map_ajax($form, $form_state) {
  return $form['studyLocation']['map-button'];
}
