<?php

/**
 * @file
 * Helper functions use at any step of TPPS form.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/form');
 */

/**
 * Adds buttons Back/Save/Next to the forms.
 *
 * The same buttons will be added above and below main form.
 *
 * @param array $chest
 *   Metadata where keys are:
 *   'form' array
 *     Drupal Form Array.
 *   'page' string
 *     Page number with 'page_' prefix to improve readability.
 *   'organism_number' int
 *     Organism Number.
 *   'data_type' string
 *     Study Data Type which contains:
 *       'G' for genotype,
 *       'P' for phenotype,
 *       'E' for environment.
 */
function tpps_form_add_buttons(array $chest) {
  $form = &$chest['form'];
  $page = $chest['page'];
  $scope_list = [
    'header' => -1000,
    'footer' => 1000,
  ];

  foreach ($scope_list as $scope => $weight) {
    $form[$scope] = [
      '#type' => 'fieldset',
      '#tree' => FALSE,
      '#weight' => $weight,
    ];
    if ($page != 'summary') {
      // @todo [VS] Use Drupal Form API 'required' attribute.
      $form[$scope]['input_description'] = [
        '#markup' => '<div class="input-description">* : Required Field</div>',
        '#weight' => -($weight),
      ];
    }
    switch ($page) {
      case 'page_2':
      case 'page_3':
        // Back, Save, Next.
        $form[$scope]['Back'] = ['#type' => 'submit', '#value' => t('Back')];

      default:
      case 'page_1':
        // Save, Next.
        $form[$scope]['Save'] = ['#type' => 'submit', '#value' => t('Save')];
        $form[$scope]['Next'] = ['#type' => 'submit', '#value' => t('Next')];
        break;

      case 'page_4':
        // Back, Save, Review, Submit, Pre-validate my VCF files.
        $form[$scope]['Back'] = ['#type' => 'submit', '#value' => t('Back')];
        $form[$scope]['Save'] = ['#type' => 'submit', '#value' => t('Save')];
        $form[$scope]['submit'] = [
          '#type' => 'submit',
          '#value' => t('Review and Submit'),
        ];

        // Pre-validate VCF.
        if (!empty($chest['data_type']) && preg_match('/G/', $chest['data_type'])) {
          $pre_valid_states = [];
          for ($i = 1; $i <= $chest['organism_number']; $i++) {
            $pre_valid_states["input[name=\"organism-{$i}[genotype][SNPs][file-type][VCF]\"]"]
              = ['checked' => FALSE];
          }
          $form[$scope]['pre_validate'] = [
            '#type' => 'button',
            '#value' => t('Pre-validate my VCF files'),
            '#states' => ['invisible' => $pre_valid_states],
            '#suffix' => '<div>Sometimes large VCF files can cause problems '
              . 'during the validation step. If your VCF file is very large, '
              . 'you can click the "Pre-validate my VCF files" button to '
              . 'validate your VCF file in the background, before clicking '
              . '"Review Information and Submit".</div>',
          ];
          $form[$scope]['Back']['#prefix'] = "<div id=\"pre-validate-message\"></div>";
        }
        break;

      case 'summary':
        // Back, Submit.
        $form[$scope]['Back'] = ['#type' => 'submit', '#value' => t('Back')];
        $form[$scope]['Next'] = ['#type' => 'submit', '#value' => t('Submit')];
        break;
    }
  }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Validation.

/**
 * Checks if required field is empty and shows error message.
 *
 * @param array $form_state
 *   Drupal Form API array with submitted values.
 * @param array $path
 *   Path to field form $form_state['complete form'] or $form.
 *   For example:
 *   ['organism-1', 'genotype', 'files', 'marker-type'].
 *
 * @return bool
 *   Returns TRUE if required field is empty and FALSE otherwise.
 */
function tpps_is_required_field_empty(array $form_state, array $path) {
  $field = drupal_array_get_nested_value($form_state['complete form'], $path);
  if (empty($field)) {
    watchdog('tpps', "Field wasn't found in 'complete form'. Path: @path",
      ['@path' => implode('/', $path)], WATCHDOG_ERROR
    );
    return FALSE;
  }
  $value = drupal_array_get_nested_value(
    $form_state['values'], tpps_get_field_path($field, $path)
  );
  if (empty($value)) {
    tpps_form_error_required($form_state, $path);
    return TRUE;
  }
  return FALSE;
}

/**
 * Shows form_validation error message that field is required.
 *
 * Field title will be detected automatically.
 * If field uses '#parents' then full path (not '#parents' must be specified).
 *
 * @param array $form_state
 *   Drupal Form API array with submitted values.
 * @param array $parents
 *   Path to field. For example:
 *   ['organism-1', 'genotype', 'SNPs', 'file-type'].
 */
function tpps_form_error_required(array $form_state, array $parents) {
  tpps_form_error($form_state, $parents, 'field is required.');
}

/**
 * Shows validation error message for given form element.
 *
 * Field's value will not be checked. Use:
 *   tpps_is_required_field_empty() to check value.
 * Field title will be detected automatically.
 *
 * @param array $form_state
 *   Drupal Form API array with submitted values.
 * @param array $path
 *   Path to field from $form_state['complete form'].
 *   (The same will be from $form). Using fields configuration data we could
 *   get all necessary data.
 *   WARNING: Not path from $form_state['values'] or $form_state['input']
 *   which could be different because of using '#parents' and '#tree' options.
 *   For example:
 *   - ['organism-1', 'genotype', 'SNPs', 'file-type'];
 *   - ['publication', 'publication_doi'];
 *   See https://www.drupal.org/docs/7/api/form-api/tree-and-parents
 * @param string $message
 *   Error message text.
 */
function tpps_form_error(array $form_state, array $path, $message) {
  $field = drupal_array_get_nested_value($form_state['complete form'], $path);
  if (empty($field['#title'])) {
    $message = t("Field didn't found at form. Path: @path",
      ['@path' => implode(' / ', $path)]
    );
    watchdog('tpps', $message, NULL, WATCHDOG_ERROR);
    drupal_set_message($message, 'error');
    return;
  }
  // Remove colon ':' and required field label.
  $title = strtok($field['#title'], ':');
  $form_path = implode('][', tpps_get_field_path($field, $path));
  form_set_error($form_path,
    t('@title: @message', ['@title' => $title, '@message' => $message])
  );
}

/**
 * Gets field's path from $form.
 *
 * This path could be used to highlight form element when validation failed,
 * to get submitted value ($form_state['values'] or $form_state['input'])
 * but can't be used to get field's configuration from $form['completed form']
 * which has unmodified by '#tree' or '#parents' path.
 *
 * @param array $field
 *   Drupal Form API Field's data.
 * @param array $path
 *   Path to field form $form_state['complete form'] or $form.
 *   For example:
 *   ['organism-1', 'genotype', 'files', 'marker-type'].
 *
 * @return array
 *   Returns array with path to field.
 */
function tpps_get_field_path(array $field, array $path) {
  return $field['#parents'] ?? ($field['#tree'] ? $path : $field['#name']) ?? [];
}

/**
 * Adds the Status Bar to the top of the TPPSc form.
 *
 * The navigation bar will be shown inside form, allows to detect
 * current step and switch to already completed steps (move back).
 *
 * @param array $form
 *   The form being populated.
 * @param array $form_state
 *   The state of the form being populated.
 */
function tpps_form_navigation_bar(array &$form, array $form_state) {
  $current_page = $form_state['stage'] ?? 1;

  // @TODO Each stage has '#cleared' option which indicates that form step
  // passed validation when it's set and value is TRUE. Remove for TPPSc.
  $page_list = [
    '1' => t('Publication Information'),
    '2' => t('Experimental Design'),
    '3' => t('Data'),
    '4' => t('Review and Submit'),
  ];
  foreach ($page_list as $number => $label) {
    $options['query'] = ['page' => $number];
    if ($current_page == $number) {
      $options['attributes']['class'] = ['current_page'];
    }
    $markup = ($markup ?? '') . l($label, 'tppsc', $options);
  }
  $form['navigation_bar'] = [
    '#markup' => '<div class="navigation-bar">' . $markup . '</div>',
    '#weight' => -1000,
  ];
}

/**
 * Get number of organisms.
 *
 * Note: Could be used when TPPS Page 1 submitted.
 *
 * @param array $state
 *   Drupal Form API State array.
 *
 * @return mixed
 *   Returns integer number of organisms.
 *   Returns NULL if TPPS Page 1 wasn't yet submitted.
 */
function tpps_form_get_organism_number(array $state) {
  return ($state['saved_values'][TPPS_PAGE_1]['organism']['number'] ?? NULL);
}

/**
 * Searches for a field of given type in array.
 *
 * @param array $array
 *   Drupal Form API array.
 *   This could be a $form or $form_state['complete form']
 *   or any other form array.
 * @param string $field_type
 *   Type of Form API field to be found in given $array.
 *
 * @return array
 *   Returns list of parents to element of requested type.
 *   Returns empty array if no elements found.
 *   Each item of array is a list of parents which could be used by
 *   drupal_array_get_nested_value() and similar.
 *
 *   Example of returned array:
 *   Array
 *   (
 *        [0] => Array
 *            (
 *                [0] => organism-1
 *                [1] => genotype
 *                [2] => tripal_fasta
 *                [3] => file
 *                [4] => file_upload
 *            )
 *
 *         [1] => Array
 *             (
 *                [0] => organism-1
 *                [1] => genotype
 *                [2] => files
 *                [3] => snps-assay
 *           )
 *     ....
 *   );
 */
function tpps_form_search_field(array $array, $field_type = 'managed_file') {
  $result = [];
  if (empty($field_type)) {
    return $result;
  }
  foreach ($array as $key => $value) {
    $type = ($value['#type'] ?? NULL);
    if (substr($key, 0, 1) == '#' || empty($type)) {
      // Elements with leading '#' in name are for internal use Drupal Form API.
      continue;
    }
    if (in_array($type, ['fieldset', 'container'])) {
      foreach (call_user_func(__FUNCTION__, $value, $field_type) as $item) {
        $result[] = array_merge([$key], $item);
      }
    }
    // Check if element is 'file'.
    elseif ($type == $field_type) {
      $result[] = [$key];
    }
  }
  return $result;
}

/**
 * Adds selectbox with 'Yes/No' options to the form.
 *
 * @param array $chest
 *   Associative array with metadata to build the field. Keys are:
 *   'form' array
 *      Required. Drupal Form Array. E.g., &$form.
 *   'form_state' array
 *      Optional. Drupal Form Array. E.g., &$form_state.
 *   Element with prefix '#' which are Drupal Form API fields.
 *   See https://api.drupal.org/api/drupal/developer%21topics%21forms_api_reference.html/7.x
 *     'field_name' string
 *        Unique name for the field. Note: it's not '#name'.
 *
 *   Required:
 *
 *     '#title' string
 *        Localized field title.
 *   Optional: '#default_value', '#required', '#state' and etc.
 *     '#name' string
 *        Unique name in $form array and HTML 'name' attribute.
 */
function tpps_form_add_yesno_field(array $chest) {

  // Validate required elements.
  if (empty($chest['form'])) {
    watchdog('tpps', 'Empty $form received', [], WATCHDOG_ERROR);
    return;
  }
  else {
    $form = &$chest['form'];
  }
  if (empty($chest['field_name']) || empty($chest['#title'])) {
    watchdog('tpps', 'Empty "name" or "#title" received', [], WATCHDOG_ERROR);
    return;
  }
  if (!isset($chest['organism_id'])) {
    return;
  }
  $field_name = $chest['field_name'];
  $i = $chest['organism_id'];

  $organism_name = 'organism-' . $i;

  // $form_state is optional and used for default value only.
  $form_state = &$chest['form_state'] ?? [];
  unset($chest['form_state']);
  // Default value.
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4] ?? NULL;
  // Options.
  // Machine names are in lower case to indicate that it's not human readable
  // name which could be localized.
  $options = ['yes' => t('Yes'), 'no' => t('No')];
  if (!($chest['#required'] ?? FALSE)) {
    $options = [0 => t('- Select -')] + $options;
  }
  // Build field using defaults.
  $defaults = [
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => ($chest['page4_values'][$field_name] ?? NULL),
    '#tree' => TRUE,

  ];
  $field = array_filter($chest, function ($e) {
    return strpos($e, '#') !== FALSE;
  }, ARRAY_FILTER_USE_KEY);
  $field = array_merge($defaults, $field);
  $parents = array_merge(($chest['parents'] ?? []), [$field_name]);
  drupal_array_set_nested_value($form, $parents, $field);
}

/**
 * Adds form fieldset for reports.
 *
 * Used by TPPS Submission Forms.
 *
 * @param array $form
 *   Drupal Form API array.
 * @param array $meta
 *   Report's metadata. Format:
 *   <unique key> => [
 *    'title' => '',
 *    'data' => '',
 *   ].
 */
function tpps_form_add_report(array &$form, array $meta) {
  if (empty($form['report'])) {
    $form['report'] = [
      '#type' => 'fieldset',
      '#title' => t('Reports'),
      '#collapsed' => FALSE,
      '#collapsible' => FALSE,
      '#weight' => 100,
      '#attributes' => ['class' => ['tpps-submission-report']],
    ];
  }
  foreach ($meta as $key => $report) {
    $key = (empty($form['report'][$key])) ? $key : md5(microtime());
    $form['report'][$key] = [
      '#title' => $report['title'] ?? $key,
      '#type' => 'fieldset',
      '#collapsed' => TRUE,
      '#collapsible' => TRUE,
      'list' => ['#markup' => $report['data'] ?? t('No data')],
    ];
  }
}

/**
 * Adds File Upload field to form.
 *
 * @param array $chest
 *   Data. Keys are:
 *   'form_state' array
 *     Submitted Form API values.
 *   'form' array
 *     Drupal Form API array with fields.
 *   'organism_id' string
 *     Organism Id. Example: 'organism-1'.
 *   'title' string
 *     Human readable name of the related file upload field
 *     (including trailing word 'File').
 *   'name' string
 *     Machine field name.
 *   'parents' array
 *     List of parents of the field.
 *
 *   Example:
 *   tpps_form_build_file_field([
 *     'form' => &$form,
 *     'parents' => [$organism_name, 'genotype', 'files'],
 *     'form_state' => $form_state,
 *     'organism_name' => $id,
 *     'field_name' => $file_field_name,
 *     'optional' => TRUE,
 *     'title' => $title,
 *     'upload_location' => 'public://' . $genotype_dir,
 *     'description' => $description,
 *     'extensions' => $extensions, // Default: ['csv tsv xlsx']
 *     'states' => $states, // Default is ''.
 *     'empty_field_value' => 'NA',
 *     // Element 'extra_elements' allow to add any not expected form elements.
 *     'extra_elements' => [],
 *     'use_fid' => FALSE, // Default is FALSE. See below.
 *     'show_extensions_in_description' => TRUE, // bool. Default is FALSE.
 *   ]);
 *
 *   List of fields which used 'fid'-related code:
 *     'snps-assay',
 *     'ssrs',
 *     'ssrs_extra'.
 *
 *   List of fields which not used 'fid'-related code:
 *     'assay-design',
 *     'snps-pop-struct',
 *     'snps-association',
 *     'snps-kinship',
 *     'indels', // was removed.
 *     'vcf',
 *     'other'.
 */
function tpps_form_build_file_field(array $chest) {
  if (empty($field_name = $chest['field_name'])) {
    return;
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Map from the $chest.
  $form = &$chest['form'];
  $form_state = $chest['form_state'] ?? [];
  $organism_name = $chest['organism_name'];
  $allow_file_reuse = $chest['allow_file_reuse'] ?? FALSE;
  $parents = $chest['parents'] ?? [];

  $title = $chest['title'] ?? '';
  $optional = $chest['optional'] ?? FALSE;
  $upload_location = $chest['upload_location'] ?? '';
  $extensions = $chest['extensions'] ?? ['csv tsv'];
  $description = $chest['description'] ?? '';
  $states = $chest['states'] ?? '';
  $empty_field_value = $chest['empty_field_value'] ?? 'NA';
  // Some fields have used this piece of code before.
  // To use: $meta['use_fid'] = TRUE; Default is FALSE.
  $use_fid = $chest['use_fid'] ?? FALSE;
  $extra_elements = $chest['extra_elements'] ?? [];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Get treasure.
  //
  // Debug Mode.
  // When enabled field's machine name will be shown in field's decription.
  $debug_mode = FALSE;
  // Main element to work with.
  $parent_element = &drupal_array_get_nested_value($form, $parents);
  // Upload folder.
  if (empty($upload_location) && ($chest['type'] ?? FALSE)) {
    $upload_location == 'public://' . variable_get(
      'tpps_' . $chest['type'] . '_files_dir', 'tpps_' . $chest['type']
    );
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add checkbox to select existing file or upload new one.
  // Note: This code is outdated and not tested. Currently not used but there
  // is a task about File Reusage and this code could became useful.
  if ($allow_file_reuse) {
    $selector_name = $chest['field_name'] . '_file_selector';
    $parent_element[$selector_name] = [
      '#type' => 'checkbox',
      '#title' => t('Reference Existing @title', ['@title' => $chest['title']]),
      '#default_value' => tpps_get_ajax_value(
        $form_state, array_merge($parents, [$selector_name])
      ),
      '#states' => $states,
    ];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // File Upload Field.
  $parent_element[$field_name] = [
    '#type' => 'managed_file',
    '#title' => $title . (!empty($optional) ? ':' : ': *'),
    '#upload_location' => $upload_location,
    '#upload_validators' => ['file_validate_extensions' => $extensions],
    '#description' => $description
    . ($debug_mode ? '<br/>Field name: <strong>' . $field_name . '</strong>' : '')
    . (($chest['show_extensions_in_description'] ?? FALSE)
      ? t('<br/>Allowed file extensions: @list',
        ['@list' => implode(', ', $extensions)]) : ''),
    '#tree' => TRUE,
    '#states' => $states,
    '#parents' => $parents,
  ];
  // dpm(print_r($parents, 1), 'parents of ' . $field_name);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Customization.
  //
  // Add extra text field for empty field value. Default is FALSE.
  if (!empty($empty_field_value)) {
    $parent_element[$field_name]['empty'] = [
      '#default_value' => $empty_field_value,
    ];
  }
  // @TODO Minor. Think about using elements with leading '#' to use them as
  // defaults for fields.
  if (!empty($extra_elements)) {
    $parent_element[$field_name] = array_merge(
      $parent_element[$field_name], $extra_elements
    );
  }
  // Some fields have used this piece of code before.
  // See function tpps_managed_file_process() which uses $fid.
  // @TODO [VS] Check if this code could be removed. It's not used on form
  // generation and when validation failed.
  if (!empty($use_fid)) {
    if (isset($parent_element[$field_name]['#value']['fid'])) {
      $parent_element[$field_name]['#default_value']
        = $parent_element[$field_name]['#value']['fid'];
    }
  }
  // Most of fields have used this code so only 2 must be excluded.
  if (!in_array($field_name, ['snps-association', 'other'])) {
    // Field 'snps-association' excluded because it didn't have this code.
    if (isset($parent_element[$field_name]['#value'])) {
      $parent_element[$field_name]['#default_value']
        = $parent_element[$field_name]['#value'];
    }
    if (
      !empty($parent_element[$field_name]['#default_value'])
      && ($file = file_load($parent_element[$field_name]['#default_value']))
    ) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      if (variable_get('tpps_genotype_file_usage_delete', TRUE)) {
        // Study Id is a number in 'TGDRXXX'.
        $study_id = substr($form_state['accession'], 4);
        file_usage_delete($file, 'tpps', 'tpps_project', $study_id);
      }
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add autocomplete field.
  // Note: This code is outdated and not tested. Currently not used but there
  // is a task about File Reusage and this code could became useful.
  if ($allow_file_reuse && 0) {
    // @TODO Update validation/submit to use '_upload' prefix.
    $autocomplete_field_name = $field_name . '_autocomplete';
    $parent_element[$autocomplete_field_name] = [
      '#type' => 'textfield',
      '#title' => t($title . ': please select an already existing '
        . 'spreadsheet with columns for the Plant ID of genotypes '
        . 'used in this study: *'),
      //'#upload_location' => $upload_location,
      '#autocomplete_path' => 'tpps/autocomplete/' . $field_name,
      '#description' => t("Please select an already existing spreadsheet "
        . "file containing SNP Genotype Assay data. The format of this "
        . "file is very important! The first column of your file should "
        . "contain plant identifiers which match the plant identifiers "
        . "you provided in your plant accession file, and all of the "
        . "remaining columns should contain SNP data."),

// @TODO Update relocated fields #names.

      '#states' => [
        'visible' => [
          ':input[name="' . $organism_name . '[genotype][SNPs][genotyping-type]"]'
            => ['!value' => 'Genotyping Assay'],
          // &&
          ':input[name="' . $organism_name . '[genotype][SNPs][file-type]"]'
            => ['!value' => 'SNP Assay file and Assay design file'],
        ],
      ],
    ];
  }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// TPPS Array

/**
 * Gets value of the nested element from the array.
 *
 * To get value by reference use:
 * $ref = &tpps_array_get_value($array, $parents);
 *
 * Shortcut for function drupal_array_get_nested_value() with default value.
 * See
 * https://api.drupal.org/api/drupal/includes%21common.inc/function/drupal_array_get_nested_value/7.x#.
 *
 * Note:
 * This function is very common but gunction tpps_get_ajax_value() is more
 * specific. It does much more and works with 'values' and 'complete form'
 * elements of given arrays.
 *
 * @param array $array
 *   Array to search.
 * @param array $parents
 *   List of the element's parents (including element name).
 *   When empty array then original $array will be returned.
 *
 * @return mixed
 *   Returns the reference (!) to the value of the element of the array.
 *   Returns $default if element wasn't found in given array but not reference.
 *   Returns $array when $parents is an empty array.
 */
function &tpps_array_get_value(array &$array, array $parents = []) {
  $value = &drupal_array_get_nested_value($array, $parents, $key_exists);
  if ($key_exists) {
    return $value;
  }
  else {
    $null = NULL;
    return $null;
  }
}

/**
 * Sets value of the nested element in the array.
 *
 * Shortcut for function drupal_array_set_nested_value().
 * See
 * https://api.drupal.org/api/drupal/includes%21common.inc/function/drupal_array_set_nested_value/7.x#.
 *
 * @param array $array
 *   Array to search.
 * @param array $parents
 *   List of the element's parents (including element name).
 *   When $parents is an empty array then $array = $value.
 * @param mixed $value
 *   The value.
 */
function tpps_array_set_value(array &$array, array $parents, $value = NULL) {
  drupal_array_set_nested_value($array, $parents, $value);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Chest

/**
 * Gets submission value by short name from the $chest.
 *
 * @param array $chest
 *   Metadata. Required keys are: 'form' and 'form_state'.
 * @param string $name
 *   Short name (synonym) of the field.
 *   Possible values are:
 *     'organism_name'
 *       Organism name. Requires 3rd argument: $organism_id.
 *       Usage: tpps_chest_get($chest, 'organism_name', $orgainsm_id).
 *     'organism_count'
 *       Number of organisms in the study.
 *       Usage: tpps_chest_get($chest, 'organism_number').
 *
 * @return mixed
 *   Returns value if element not found.
 *
 * @TODO Replace with class Chest and find better name.
 */
function &tpps_chest_get(array $chest, $name) {
  // Define NULL as variable to return it as reference.
  $null = NULL;
  // Validate.
  if (empty($chest) || empty($name)) {
    return $null;
  }
  // Prepare parents for TPPS Pages.
  for ($i = 1; $i <= 4; $i++) {
    $page_parents[$i] = ['form_state', 'saved_values', $i];
  }
  // Get parents.
  switch ($name) {
    case 'organism_name':
      if (func_num_args() < 3) {
        return $null;
      }
      $parents = array_merge($page_parents[1],
        ['organism', func_get_arg(2), 'name']
      );
      break;

    case 'organism_count':
      $parents = array_merge($page_parents[1],
        ['organism', 'number']
      );
      break;
  }

  // Get value.
  if ($parents) {
    $value = &drupal_array_get_nested_value($chest, $parents);
    return $value;
  }
  else {
    return $null;
  }
}

/**
 * Sets focus on given field by name.
 *
 * WARNING: Be sure element has no 'autofocus' attribute because it won't work.
 * Timeout is required to have element at page.
 * NOTE: Multistep forms could call this function multiple times for each
 * step but if field not found in $form extra JS code will not be added to page.
 * See js/tpps_autofocus.js file.
 *
 * @param array $form
 *   Drupal Form API Array to be updated.
 * @param string $fieldname
 *   Field name. Use key from $form array. Example: 'tpps_submission_accession'.
 *   When $fieldname is empty we just do not add JS
 *   because it's not critical feature.
 * @param array $parents
 *   Optional and not critical. Allows to remove 'autofocus' attribute.
 *
 * @TODO Minor. Use CSS id intead of name attribute to set focus.
 */
function tpps_form_autofocus(array &$form = [], $fieldname = '', array $parents = []) {
  // Delay in msec.
  $timeout = 1000;
  if (empty($fieldname)) {
    return;
  }
  if ($parents) {
    $path = array_merge($parents, [$fieldname]);
    // We need not Drupal From API Field name but HTML fieldname.
    // Example: 'publication[status]'.
    $html_fieldname = str_replace(
      $parents[0] . ']', $parents[0], implode('][', $path)
    ) . ']';
    $field = drupal_array_get_nested_value($form, $path) ?? NULL;
  }
  else {
    $html_fieldname = $fieldname;
    $field = $form[$fieldname] ?? NULL;
  }
  if ($field && !empty($html_fieldname)) {
    // Do nothing if field not found at form.
    tpps_add_css_js('autofocus', $form);
    $form['#attached']['js'][] = [
      'type' => 'setting',
      'data' => [
        'tpps' => [
          'autoFocus' => [
            'FieldName' => $html_fieldname,
            'Timeout' => $timeout,
          ],
        ],
      ],
      'scope' => 'footer',
      'group' => JS_THEME,
      'weight' => 5,
    ];
    // @TODO Minor. Field could be not on 1st level.
    unset($field['#attributes']['autofocus']);
  }
}

/**
 * Checks if current form is TPPSc form.
 *
 * @param array $form_state
 *   Drupal Form State array.
 *
 * @return bool
 *   Returns TRUE if form Id is 'tppsc_main' and FALSE otherwise.
 */
function tpps_form_is_tppsc(array $form_state) {
  return (bool) (($form_state['build_info']['form_id'] ?? 'tpps_main') == 'tppsc_main');
}

/**
 * Get study type list or one item.
 *
 * Usage example:
 * module_load_include('inc', 'tpps', 'includes/form');
 * $study_type_options = tpps_form_get_study_type($key);
 *
 * @param int $key
 *   Key in the list of options. This value used in form submitters.
 *
 * @return mixed
 *   Returns:
 *   1. $key was specified and index $key exists: translated option label
 *   2. $key was specified but index $key not exists: NULL
 *   3. $key wasn't specified (NULL): whole list of study types/
 */
function tpps_form_get_study_type($key = NULL) {
  $options = [
    0 => t('- Select -'),
    1 => t('Natural Population (Landscape)'),
    2 => t('Growth Chamber'),
    3 => t('Greenhouse'),
    4 => t('Experimental/Common Garden'),
    5 => t('Plantation'),
  ];
  return is_null($key) ? $options : ($options[$key] ?? NULL);
}

/**
 * Gets parent element and field name from the given path.
 *
 * Drupal allows to get/set nested field's values using function
 * drupal_array_get_nested_value() but to unset (or relocate) you need
 * to have parent element by reference. This function get's parent element
 * and suggests to store field name in the end of the list of parents.
 * Used in includes/submission_interface.inc.
 *
 * @param array $array
 *   Array where field will be searched.
 * @param mixed $path
 *   Path to the field format:
 *   1. Field name (string). E.g., 'values' for $array['values'].
 *   2. Field name as path (array). E.g., ['values'] for $array['values'].
 *   3. Path including field name (array). E.g., ['values', 'field_name']
 *      for $array['values']['field_name'].
 *
 * @return mixed
 *   Associative array with keys:
 *   - 'parent_element' - reference to the parent element.
 *   - 'field_name' - name of the field under the 'parent_element'.
 */
function &tpps_form_get_path(array &$array = NULL, $path = NULL) {
  if (empty($array) || empty($path)) {
    watchdog('tpps', 'Function @function received empty arguments. '
      . '<hr />Path: <pre>@path</pre>.'
      . '<hr />Array: <pre>@array</pre>',
      [
        '@function' => __FUNCTION__,
        '@array' => print_r($array, 1),
        '@path' => print_r($path, 1),
      ],
      WATCHDOG_ERROR
    );
    return NULL;
  }
  // Suppose that $path is a string with field name at the 1st level
  // of $interface array.
  $result = [
    'parent_element' => &$array,
    'field_name' => $path,
  ];
  // Check if $path is an array with field's parents.
  if (is_array($path)) {
    // When element not exists in this version of array then create it first.
    // Otherwise we can't move item to non-existing element.
    if (empty(drupal_array_get_nested_value($array, $path))) {
      drupal_array_set_nested_value($array, $path, NULL);
    }
    // Last element is always a field name.
    $result['field_name'] = end($path);
    array_pop($path);
    // Check if there is a list of parents.
    if (count($path) >= 1) {
      $result['parent_element'] = &drupal_array_get_nested_value($array, $path);
      if (empty($result['parent_element'])) {
        dpm(print_r($path, 1), 'Empty parent element!');
      }
    }
  }
  return $result;
}

/**
 * Gets Study Accession from TPPS Form State array.
 *
 * @param array $form_state
 *   Drupal Form State Array.
 * @param bool $only_number
 *   When TRUE only digits will be returns (E.g., '1234'). When FALSE (default)
 *   then whole accession will be returned (e.g., 'TGDR1234').
 *
 * @return string
 *   Returns full accession (default), only accession number or
 *   empty string if accession wasn't found in given Form State Array.
 */
function tpps_form_get_accession(array $form_state, $only_number = FALSE) {
  $accession = $form_state['accession'] ?? '';
  return (string) (
    ($only_number) ? str_replace('TGDR', '', $accession) : $accession
  );
}

/**
 * Creates the main TPPS/TPPSc form.
 *
 * Provides the form based on which step the user is on.
 *
 * @param array $form
 *   The form to be created.
 * @param array $form_state
 *   The state of the form to be created.
 *
 * @return array
 *   Returns the completed form.
 */
function tpps_form_build_main(array $form, array &$form_state, $accession = NULL) {
  global $user;

  // Check if this is TPPS or TPPSC Form using Url.
  // $form_state could have no necessary data yet when loaded first step of
  // the multistep form.
  if (isset($form_state['tpps_type'])) {
    $is_tppsc = ($form_state['tpps_type'] == 'tppsc');
  }
  else {
    $is_tppsc = (strpos($_GET['q'], 'tppsc') === 0);
  }

  if (!$is_tppsc) {
    module_load_include('php', 'tpps', 'forms/build/front');
  }
  module_load_include('inc', 'tpps', 'includes/form');
  $form = $form ?? [];
  $frontpage_values = &$form_state['saved_values']['frontpage'] ?? NULL;
  // JS and CSS.
  tpps_add_css_js('main', $form);
  // Add class to apply CSS rules differntly for TPPS and TPPSc Forms
  // CSS form id is changing because of AJAX requests so we use form's class
  // in CSS rules instead of form's id with regex.
  $form['#attributes']['class'][] = ($is_tppsc) ? 'tppsc-form' : 'tpps-form';
  // Use the same validator- and submitter-function for both forms (TPPS/TPPSc).
  $form['#validate'][] = 'tpps_main_validate';
  $form['#submit'][] = 'tpps_main_submit';

  if ($is_tppsc) {
    // User not logged in, trying to access front page.
    if (!isset($user->mail) and !isset($form_state['stage'])) {
      $form_state['stage'] = 'frontpage';

      // Create front page.
      module_load_include('php', 'tpps', 'forms/build/front');
      tpps_front_create_form($form, $form_state);
      return $form;
    }
    // User not logged in, trying to access non-front page.
    elseif (!isset($user->mail)) {
      $destination = drupal_get_destination();
      drupal_goto('user/login', array('query' => $destination));
    }
    // User logged in, trying to load form from existing accession.
    elseif (isset($accession) && !isset($form_state['stage'])) {
      $old_form_state = tpps_load_submission_state($accession);
      tpps_form_state_info($form_state, $old_form_state);
    }
    // User logged in, no accession provided.
    else {
      if (!isset($form_state['stage'])) {
        $form_state['stage'] = 'frontpage';

        // Create the front page.
        module_load_include('php', 'tpps', 'forms/build/front');
        tpps_front_create_form($form, $form_state);
        return $form;
      }
      else {
        if (
          !isset($frontpage_values['accession'])
          || ($frontpage_values['accession'] ?? NULL) === 'new'
        ) {
          // Initialize project.
          tpps_init_project($form_state);
          tpps_create_submission($form_state, $user->uid);
        }
        $old_form_state = tpps_load_submission_state($frontpage_values['accession']);
        tpps_form_state_info($form_state, $old_form_state);
      }
    }

    // Hidden form accession element.
    // Note: Adding this elements to $form heare require all sub-functions to
    // use reference and not return $form array.
    // @TODO [VS] Use common function which called from subfunctions to add
    // common elements instead of pass $form by reference.
    $form['accession'] = [
      '#type' => 'hidden',
      '#value' => $form_state['accession'],
    ];
  }
  elseif (!$is_tppsc) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // TPPS.
    // Stage/Page.
    if (!isset($form_state['stage'])) {
      if (!isset($accession)) {
        // First access, no accession in link.
        $form_state['stage'] = 'frontpage';

        tpps_front_create_form($form, $form_state);
        return $form;
      }
      else {
        // First access, and with accession in link.
        $old_form_state = tpps_load_submission_state($accession);
        if (empty($old_form_state)) {
          drupal_goto('tpps');
        }
        tpps_form_state_info($form_state, $old_form_state);
      }
    }
    else {
      // Not first access.
      if (
        !isset($frontpage_values['accession'])
        || ($frontpage_values['accession'] ?? NULL) === 'new'
      ) {
        // No submissions, or selected to create new submission.
        if (
          tpps_access('administer tpps module')
          && !empty($frontpage_values['custom_accession_check'])
        ) {
          $accession = $frontpage_values['custom_accession'];
        }
        tpps_init_project($form_state, $accession);
        tpps_create_submission($form_state, $user->uid);
      }
      $old_form_state = tpps_load_submission_state($frontpage_values['accession']);
      if (!empty($old_form_state)) {
        tpps_form_state_info($form_state, $old_form_state);
      }
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (!isset($form_state['dbxref_id']) or !isset($form_state['accession'])) {
    tpps_init_project($form_state);
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission Form Versions.
  // Set version for 'Submission Interface' feature.
  // Note: We need to set version before 'update' version.
  module_load_include('inc', 'tpps', 'includes/submission_form_version');
  tpps_submission_form_version_set($form, $form_state);

  // Upgrade to latest Submission Form Version.
  $version = tpps_submission_form_version_get($form_state);
  $latest_version = TPPS_SUBMISSION_FORM_CURRENT_VERSION;
  if ($version < $latest_version) {
    // Clone existing $form_state.
    $new_form_state = $form_state;
    for (; $version <= $latest_version; $version++) {
      tpps_submission_form_version_upgrade($form_state, $version, $new_form_state);
      $new_form_state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY] = $version;
    }
    // Update only if version was really upgraded.
    tpps_update_submission_state($new_form_state);

    if (
      (TPPS_SUBMISSION_FORM_START_VERSION + 1) != $latest_version
      && tpps_is_si_debug_mode()
    ) {
      $message = 'Submission Form Version was upgraded from @version to @latest_version.';
      $params = [
        '@version' => TPPS_SUBMISSION_FORM_START_VERSION + 1,
        '@latest_version' => $latest_version,
      ];
      tpps_message($message, $params, 'status');
    }
    // Restore $form_state.
    $form_state = $new_form_state;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Must be after tpps_init_project() which sets accession for new studies.
  $accession = $form_state['accession'] ?? $frontpage_values['accession'] ?? NULL;
  $form['#attached']['js'][] = [
    'data' => [
      'tpps' => [
        'stage' => $form_state['stage'] ?? NULL,
        'accession' => $accession,
      ],
    ],
    'type' => 'setting',
  ];
  // Hidden form accession element.
  // Note: Adding this elements to $form heare require all sub-functions to
  // use reference and not return $form array.
  $form['accession'] = ['#type' => 'hidden', '#value' => $accession];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (!$is_tppsc) {
    $form_state['tpps_type'] = 'tpps';

    module_load_include('inc', 'tpps', 'includes/status_bar');
    tpps_status_bar($form, $form_state);

    $page1_values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];
    if (!isset($page1_values['primaryAuthor'])) {
      $contact_bundle = tripal_load_bundle_entity(['label' => 'Tripal Contact Profile']);
      if ($contact_bundle) {
        $query = new EntityFieldQuery();
        $results = $query->entityCondition('entity_type', 'TripalEntity')
          ->entityCondition('bundle', $contact_bundle->name)
          ->fieldCondition('local__email', 'value', $user->mail)
          ->range(0, 1)
          ->execute();
        if (!empty($results['TripalEntity'])) {
          $entity = current(array_reverse(entity_load('TripalEntity',
            array_keys($results['TripalEntity']))));
          $form_state['saved_values'][TPPS_PAGE_1]['primaryAuthor']
            = $form_state['saved_values'][TPPS_PAGE_1]['primaryAuthor'] ?? $entity->title;
        }
      }
    }
  }
  elseif ($is_tppsc) {
    $form_state['tpps_type'] = 'tppsc';
    tpps_update_submission_state($form_state);
    if (variable_get('tpps_top_bar', 'status') == 'status') {
      module_load_include('inc', 'tpps', 'includes/status_bar');
      tpps_status_bar($form, $form_state);
    }
    else {
      tpps_form_navigation_bar($form, $form_state);
    }
  }

  switch ($form_state['stage']) {
    case TPPS_PAGE_1:
      module_load_include('php', 'tpps', 'forms/build/page_1');
      tpps_page_1_create_form($form, $form_state);
      break;

    case TPPS_PAGE_2:
      module_load_include('php', 'tpps', 'forms/build/page_2');
      tpps_page_2_create_form($form, $form_state);
      break;

    case TPPS_PAGE_3:
      module_load_include('php', 'tpps', 'forms/build/page_3');
      tpps_page_3_create_form($form, $form_state);
      break;

    case TPPS_PAGE_4:
      module_load_include('php', 'tpps', 'forms/build/page_4');
      tpps_page_4_create_form($form, $form_state);
      break;

    case 'summarypage':
      module_load_include('php', 'tpps', 'forms/build/summary');
      tpps_summary_create_form($form, $form_state);
      break;

    default:
      drupal_set_message(t('Invalid form stage.'), 'error');
      break;
  }
  return $form;
}
