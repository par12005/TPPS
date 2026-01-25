<?php

/**
 * @file
 * Shortcuts to get data from the $form_state array.
 *
 * WARNING:
 * ONLY for latest version of Form State (not for Shared State).
 * So must be updated to use latest Submission Version.
 *
 * @todo Create instance of this class in hook_init() (or similar) and make it
 * global. But having it global means that there must be only one $form_state
 * but we also have 'Submission Shared State', 'Submission Form State' and
 * currently processed $form_state. Create 3 items ($SSS, $FS, $SFS)?
 *   $GLOBALS['FS'] = new FormState();
 *
 * @todo Use file field class's method ::getParents() to find elemnts and have
 * SubmissionVersion support.
 * @todo Shoud default values for getters be used?
 * @todo Minor. Remove dependency from 'TppsArray' class but it means code
 * duplication.
 *
 * Usage example:
 * $formState = new FormState($form_state);
 * $formState->getAccession();
 * $formState->set(['stage'], 2);
 *
 * Using shortcuts:
 * To get $form_state['status']:
 *   $fs->status;
 *   // or
 *   $fs->get(['status']);
 * To get $form_state['saved_values']:
 *   $fs->saved_values();
 * To get sub-element of the nested array:
 *   $fs->saved_values(['dbxref_id']);
 * Some often used elements have one methods:
 *  $fs->getOrganismNumber();
 *
 * Naming rules:
 *   $formState - currently processing form.
 *   $sharedState - Submission's Shared State.
 *   @todo Find better name.
 *   $state - Submission Form State.
 */

/**
 * Gets TPPS Submission specific data from given Drupal Form State array.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class FormState {

  /**
   * Submission's State Array.
   *
   * See __set()/__get().
   *
   * @var array
   */
  protected $formState = [];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Plain items on 1st level.
  // @todo Add description.
  protected $stage;
  protected $accession;
  protected $dbxref_id;
  protected $tpps_type;
  protected $status;
  protected $updated;
  protected $created;
  protected $approved;
  protected $completed;
  protected $loaded;
  protected $submitting_uid;
  protected $job_id;
  protected $revised_files;
  protected $admin_comments;
  protected $alternative_accessions;
  protected $data;
  protected $has_file_element;

  /**
   * Class constructor.
   *
   * @param array $form_state
   *   TPPS Submission Form State.
   */
  public function __construct(array $form_state = NULL) {
    if (!empty($form_state)) {
      $this->formState = $form_state;
      // Fill items on 1st level to allow shorter references.
      // E.g., $fs->status instead of $fs->get(['status']).
      $items = [
        'stage',
        'accession',
        'dbxref_id',
        'tpps_type',
        'status',
        'updated',
        'created',
        'approved',
        'completed',
        'loaded',
        'submitting_uid',
        'job_id',
        'revised_files',
        'admin_comments',
        'alternative_accessions',
        'data',
        'has_file_element',
      ];
      foreach ($items as $name) {
        if (isset($form_state[$name])) {
          $this->$name = $form_state[$name];
        }
      }
    }
    return $this;
  }

  /**
   * Magic setter.
   *
   * @param string $name
   *   Class property name.
   * @param mixed $value
   *   Value to be set.
   */
  public function __set($name, $value) {
    // Allow to overwrite magic with custom setter.
    $func = 'set' . ucfirst($name);
    if (method_exists($this, $func)) {
      $this->$func($value);
    }
    else {
      if (property_exists($this, $name)) {
        $this->$name = $value;
      }
      else {
        throw new InexistentPropertyException(
          t('Inexistent property: @name', ['@name' => $name])
        );
      }
    }
    return $this;
  }

  /**
   * Magic getter.
   *
   * @param string $name
   *   Class property name.
   *
   * @return mixed
   *   Returns property value.
   */
  public function &__get($name) {
    // Allow to overwrite magic with custom getter.
    $func = 'get' . ucfirst($name);
    if (method_exists($this, $func)) {
      return $this->$func();
    }
    else {
      // Property.
      if (property_exists($this, $name)) {
        return $this->$name;
      }
      else {
        throw new Exception("Inexistent property: $name");
      }
    }
    return $this;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Shortcuts for some $formState elements.

  /**
   * Gets sub-element under $formState['saved_values'].
   *
   * @param array $parents
   *   Path to the element.
   *
   * @return mixed
   *   Returns value of the sub-element.
   *   Returns NULL if element doesn't exists.
   *   Returns all sub-elements when $parents is empty.
   */
  public function saved_values(array $parents = []) {
    return $this->get(array_merge([__FUNCTION__], $parents));
  }

  /**
   * Gets sub-element under $formState['values'].
   *
   * @param array $parents
   *   Path to the element.
   *
   * @return mixed
   *   Returns value of the sub-element.
   *   Returns NULL if element doesn't exists.
   *   Returns all sub-elements when $parents is empty.
   */
  public function values(array $parents = []) {
    return $this->get(array_merge([__FUNCTION__], $parents));
  }

  /**
   * Gets sub-element under $formState['input'].
   *
   * @param array $parents
   *   Path to the element.
   *
   * @return mixed
   *   Returns value of the sub-element.
   *   Returns NULL if element doesn't exists.
   *   Returns all sub-elements when $parents is empty.
   */
  public function input(array $parents = []) {
    return $this->get(array_merge([__FUNCTION__], $parents));
  }

  /**
   * Gets sub-element under $formState['stats'].
   *
   * @param array $parents
   *   Path to the element.
   *
   * @return mixed
   *   Returns value of the sub-element.
   *   Returns NULL if element doesn't exists.
   *   Returns all sub-elements when $parents is empty.
   */
  public function stats(array $parents = []) {
    return $this->get(array_merge([__FUNCTION__], $parents));
  }

  /**
   * Gets sub-element under $formState['ids'].
   *
   * @param array $parents
   *   Path to the element.
   *
   * @return mixed
   *   Returns value of the sub-element.
   *   Returns NULL if element doesn't exists.
   *   Returns all sub-elements when $parents is empty.
   */
  public function ids(array $parents = []) {
    return $this->get(array_merge([__FUNCTION__], $parents));
  }

  /**
   * Gets sub-element under $formState['file_info'].
   *
   * @param array $parents
   *   Path to the element.
   *
   * @return mixed
   *   Returns value of the sub-element.
   *   Returns NULL if element doesn't exists.
   *   Returns all sub-elements when $parents is empty.
   */
  public function file_info(array $parents = []) {
    return $this->get(array_merge([__FUNCTION__], $parents));
  }

  /**
   * Gets sub-element under $formState['clicked_button'].
   *
   * @param array $parents
   *   Path to the element.
   *
   * @return mixed
   *   Returns value of the sub-element.
   *   Returns NULL if element doesn't exists.
   *   Returns all sub-elements when $parents is empty.
   */
  public function clicked_button(array $parents = []) {
    return $this->get(array_merge([__FUNCTION__], $parents));
  }

  // Common methods.
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Gets value of the arbitrary element using list of parents.
   *
   * @param array $parents
   *   List of parents including element's name.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public function get(array $parents) {
    $this->validate();
    dpm(print_r($parents, 1));
    return TppsArray::get($this->formState, $parents);
  }

  /**
   * Sets value of the arbitrary element using list of parents.
   *
   * @param array $parents
   *   List of parents including element's name.
   * @param mixed $value
   *   Value of the element.
   */
  public function set(array $parents, $value) {
    $this->validate();
    return TppsArray::set($this->formState, $parents, $value);
  }

  /**
   * Checks if property 'formState' was set and not empty.
   *
   * @throws \Exception
   *   Throws exception when property 'formState' wasn't set or empty.
   */
  public function validate() {
    if (empty($this->formState)) {
      $message = 'Empty Form State';
      throw new Exception($message);
    }
  }

  // Custom methods.
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Checks if each organism has separate accession file.
   *
   * Checkbox on Page 3 with title:
   * I would like to upload a separate plant accession file for each species.
   *
   * @return bool
   *   Returns TRUE when each organism has own accession file.
   *   Returns FALSE otherwise (single accession file for all organisms).
   */
  public function getAccessionCheck() {
    $parents = ['values', 'tree-accession', 'check'];
    return (bool) $this->get($parents);
  }

  /**
   * Checks if study has any phenotypic data for given organism.
   *
   * Checkbox with title:
   * My phenotypes include traits and/or environmental information other than
   * mass spectrometry or isotope analysis.
   *
   * @param int $organism_index
   *   Ordinal organism number on page. Default is 1.
   *   Default value could be used to check if study has phenotypes in common.
   *
   * @return bool
   *   Returns TRUE if organism has any phenotype data and FALSE otherwise.
   */
  public function hasAnyPhenotypeData($organism_index = 1) {
    $organism_key = 'organism-' . $organism_index;
    $parents = ['values', $organism_key, 'phenotype', 'normal-check'];
    return (bool) $this->get($parents);
  }

  /**
   * Ghecks if study has phenotype metadata file for given organism.
   *
   * Checkbox with title:
   * I would like to upload a phenotype metadata file'),
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   *
   * @return bool
   *   Returns TRUE if organism has phenotype metadata file and FALSE otherwise.
   */
  public function hasPhenotypeMetaDataFile($organism_index) {
    $organism_key = 'organism-' . $organism_index;
    $parents = ['values', $organism_key, 'phenotype', 'check'];
    return (bool) $this->get($parents);
  }

  /**
   * Gets study's accession. E.g., 'TGDR1234'.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public function getAccession() {
    $parents = ['accession'];
    return $this->get($parents);
  }

  /**
   * Gets study's accession number. E.g., '1234'.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public function getAccessionNumber() {
    // Using 'str_replace' seems more secure then just cut off first 4 chars:
    // substr($this->getAccession(), 4).
    return str_replace('TGDR', '', $this->getAccession());
  }

  /**
   * Gets value of the triggering element.
   *
   * For buttons it will be button's label.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public function getTriggeringElement() {
    $parents = ['triggering_element', '#value'];
    return $this->get($parents);
  }

  /**
   * Gets number of organisms.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public function getOrganismNumber() {
    $parents = ['saved_values', TPPS_PAGE_1, 'organism', 'number'];
    return $this->get($parents);
  }

  /**
   * Gets number of species.
   *
   * @return mixed
   *   Returns value or NULL.
   *
   * @todo Probably the same as getOrganismNumber().
   */
  public function getSpeciesNumber() {
    $parents = ['stats', 'species_count'];
    return $this->get($parents);
  }

  /**
   * Gets page key.
   *
   * @return mixed
   *   Returns integer value for Pages 1-4 and string for welcome page and
   *   summary page.
   */
  public function getPage() {
    $parents = ['stage'];
    return $this->get($parents);
  }

  /**
   * Gets form id.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public function getFormId() {
    $parents = ['build_info', 'form_id'];
    return $this->get($parents);
  }

  /**
   * Gets Import Metadata.
   *
   * Imported Studies has some extra information about export/import process.
   *
   * @return array
   *   Returns Submission Import Meta with some useful extra items.
   */
  public function getImportMeta() {
    $output = [];
    if (
      empty($this->formState)
      || empty($this->formState['saved_values']['tpps_submission_import'])
    ) {
      return $output;
    }
    $import_meta = $this->formState['saved_values']['tpps_submission_import'];

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
    $import_meta['current_version'] = SubmissionVersion::get($this->form_state);

    return $import_meta;
  }

  /**
   * Gets name of the specific organism.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public function getOrganismName($organism_index) {
    $parents = [
      'saved_values', TPPS_PAGE_1, 'organism', $organism_index, 'name',
    ];
    return $this->get($parents);
  }

  /**
   * Gets name of the specific organism.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param int $phenotype_index
   *   Ordinal number of phenotype at page.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public function getPhenotypeName($organism_index, $phenotype_index) {
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
    return $this->get($parents);
  }

  /**
   * Gets name or value of the clidded button.
   *
   * @param string $element
   *   Name of the sub-element of the $this->formState['triggering_element'].
   *   WARNING:
   *   Use '#name' if possible because '#value' is a translatable string.
   *   Possible values are:
   *     '#type',
   *     '#value',
   *     '#name',
   *     '#input',
   *     '#button_type',
   *     '#executes_submit_callback',
   *     '#limit_validation_errors',
   *     '#process',
   *     '#theme_wrappers',
   *     '#after_build',
   *     '#defaults_loaded',
   *     '#tree',
   *     '#parents',
   *     '#array_parents',
   *     '#weight',
   *     '#processed',
   *     '#required',
   *     '#attributes',
   *     '#title_display',
   *     '#id'.
   *
   * @return mixed
   *   Returns value of the sub-element or NULL if element doesn't exists.
   */
  public function getClickedButton($element = '#name') {
    return $this->clicked_button([$element]);
  }

}
