<?php

/**
 * @file
 * Shortcuts to get data from the $form_state array.
 *
 * WARNING:
 * ONLY for latest version of Form State (not for Shared State).
 * So must be updated to use latest Submission Version.
 *
 * @todo Use other class's ::getParents() methods to get latest version values.
 * @todo Shoud default values for getters be used?
 * @todo Use dynamic methods to avoid sending $form_state each time.
 * Problems to solve:
 * 1) What instance name should be used to avoid naming conflicts and
 * keep readability? E.g., $fs = new FormState($form_state);
 * 2) This will bring the same data cloned in memory multiple times.
 */

/**
 * Gets TPPS Submission specific data from given Drupal Form State array.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class FormState {

  // Custom methods.
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Checks if each organism has separate accession file.
   *
   * Checkbox on Page 3 with title:
   * I would like to upload a separate plant accession file for each species.
   *
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return bool
   *   Returns TRUE when each organism has own accession file.
   *   Returns FALSE otherwise (single accession file for all organisms).
   */
  public static function getAccessionCheck(array $form_state) {
    $parents = ['values', 'tree-accession', 'check'];
    return (bool) static::get($form_state, $parents);
  }

  /**
   * Checks if study has any phenotypic data for given organism.
   *
   * Checkbox with title:
   * My phenotypes include traits and/or environmental information other than
   * mass spectrometry or isotope analysis.
   *
   * @param array $form_state
   *   Drupal Form State array.
   * @param int $organism_index
   *   Ordinal organism number on page. Default is 1.
   *   Default value could be used to check if study has phenotypes in common.
   *
   * @return bool
   *   Returns TRUE if organism has any phenotype data and FALSE otherwise.
   */
  public static function hasAnyPhenotypeData(array $form_state, $organism_index = 1) {
    $organism_key = 'organism-' . $organism_index;
    $parents = ['values', $organism_key, 'phenotype', 'normal-check'];
    return (bool) static::get($form_state, $parents);
  }

  /**
   * Ghecks if study has phenotype metadata file for given organism.
   *
   * Checkbox with title:
   * I would like to upload a phenotype metadata file'),
   *
   * @param array $form_state
   *   Drupal Form State array.
   * @param int $organism_index
   *   Ordinal organism number on page.
   *
   * @return bool
   *   Returns TRUE if organism has phenotype metadata file and FALSE otherwise.
   */
  public static function hasPhenotypeMetaDataFile(array $form_state, $organism_index) {
    $organism_key = 'organism-' . $organism_index;
    $parents = ['values', $organism_key, 'phenotype', 'check'];
    return (bool) static::get($form_state, $parents);
  }

  /**
   * Gets study's accession. E.g., 'TGDR1234'.
   *
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public static function getAccession(array $form_state) {
    $parents = ['accession'];
    return static::get($form_state, $parents);
  }

  /**
   * Gets study's accession number. E.g., '1234'.
   *
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public static function getAccessionNumber(array $form_state) {
    // Using 'str_replace' seems more secure then just cut off first 4 chars:
    // substr(static::getAccession($form_state), 4).
    return str_replace('TGDR', '', static::getAccession($form_state));
  }

  /**
   * Gets value of the triggering element.
   *
   * For buttons it will be button's label.
   *
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public static function getTriggeringElement(array $form_state) {
    $parents = ['triggering_element', '#value'];
    return static::get($form_state, $parents);
  }

  /**
   * Gets number of organisms.
   *
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public static function getOrganismNumber(array $form_state) {
    $parents = ['saved_values', TPPS_PAGE_1, 'organism', 'number'];
    return static::get($form_state, $parents);
  }

  /**
   * Gets number of species.
   *
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns value or NULL.
   *
   * @todo Probably the same as getOrganismNumber().
   */
  public static function getSpeciesNumber(array $form_state) {
    $parents = ['stats', 'species_count'];
    return static::get($form_state, $parents);
  }

  /**
   * Gets page key.
   *
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns integer value for Pages 1-4 and string for welcome page and
   *   summary page.
   */
  public static function getPage(array $form_state) {
    $parents = ['stage'];
    return static::get($form_state, $parents);
  }

  /**
   * Gets form id.
   *
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public static function getFormId(array $form_state) {
    $parents = ['build_info', 'form_id'];
    return static::get($form_state, $parents);
  }

  // Common methods.
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Gets value of the arbitrary element using list of parents.
   *
   * @param array $form_state
   *   Drupal Form State array.
   * @param array $parents
   *   List of parents including element's name.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public static function get(array $form_state, array $parents) {
    return TppsArray::getValue($form_state, $parents);
  }

  /**
   * Sets value of the arbitrary element using list of parents.
   *
   * @param array $form_state
   *   Drupal Form State array.
   * @param array $parents
   *   List of parents including element's name.
   * @param mixed $value
   *   Value of the element.
   */
  public static function set(array $form_state, array $parents, $value) {
    return TppsArray::setValue($form_state, $parents, $value);
  }

  /**
   * Gets Import Metadata.
   *
   * Imported Studies has some extra information about export/import process.
   *
   * @return array
   *   Returns Submission Import Meta with some useful extra items.
   */
  public static function getImportMeta(array $form_state) {
    $output = [];
    if (
      empty($form_state)
      || empty($form_state['saved_values']['tpps_submission_import'])
    ) {
      return $output;
    }
    $import_meta = $form_state['saved_values']['tpps_submission_import'];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Sanitize data.
    foreach ($import_meta as $key => $value) {
      if ($safe_key = check_plain(($key ?? '')) != $key) {
        watchdog('tpps', "Submission Import Metadata value had not allowed "
          . "symbols which was sanitized.", [], WATCHDOG_NOTICE);
        unset($import_meta[$key]);
      }
      if ($safe_value = check_plain(($value ?? '')) != $value) {
        watchdog('tpps', "Submission Import Metadata value had not allowed "
          . "symbols which was sanitized.", [], WATCHDOG_NOTICE);
        unset($import_meta[$key]);
        $import_meta[$safe_key] = $safe_value;
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Add some useful items and default values.
    $import_meta['site'] = $import_meta['site'] ?? 'https://treegenesdb.org';
    $import_meta['accession_link'] = ($import_meta['site'])
      ? l($import_meta['original_accession'], $import_meta['site']
        . '/tpps/submission/' . $import_meta['original_accession'] . '/view')
        : $import_meta['original_accession'];
    // Full data: Human readable and timestamp.
    $import_meta['full_date'] = $import_meta['imported_date'] ?? '';
    if (variable_get('tpps_imported_study_report_show_timestamp', FALSE)) {
      $import_meta['full_date'] .= (empty($import_meta['imported']) ? ''
        : ' [' . $import_meta['imported'] . ']');
    }
    $import_meta['current_version'] = SubmissionVersion::get($form_state);

    return $import_meta;
  }

  /**
   * Gets name of the specific organism.
   *
   * @param array $form_state
   *   Drupal Form State array.
   * @param int $organism_index
   *   Ordinal number of organism at page.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public static function getOrganismName(array $form_state, $organism_index) {
    $parents = [
      'saved_values', TPPS_PAGE_1, 'organism', $organism_index, 'name',
    ];
    return static::get($form_state, $parents);
  }

  /**
   * Gets name of the specific organism.
   *
   * @param array $form_state
   *   Drupal Form State array.
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param int $phenotype_index
   *   Ordinal number of phenotype at page.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public static function getPhenotypeName(array $form_state, $organism_index, $phenotype_index) {
    $parents = [
      'saved_values',
      TPPS_PAGE_4,
      'organism-' . $organism_index,
      'phenotype',
      'phenotypes-meta',
      $phenotype_index,
      // There are also fields which could be used:
      // synonym_name: "Annual Precipitation",
      // synonym_description: "mean annual precipitation of that year",
      // synonym_id: "0",
      // name: "Annual Precipitation",
      // env-check: 1,
      // attribute: "0",
      // attr-other: "",
      // description: "mean annual precipitation of that year",
      // unit: "130439",
      // unit-other: "",
      // structure: "52336",
      // struct-other: "".
      'name',
    ];
    return static::get($form_state, $parents);
  }

}
