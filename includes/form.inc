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
 * @param array $form_bus
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
function tpps_form_add_buttons(array $form_bus) {
  $form = &$form_bus['form'];
  $page = $form_bus['page'];
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
        $form[$scope]['Back'] = [
          '#type' => 'submit',
          '#value' => t('Back'),
          // Disable validation.
          // Currently useless because all fields are optional but this
          // could be changed in future. But it removed saved data of stage
          // when 'Back' button clicked which is not good.
          //'#limit_validation_errors' => [],
        ];

      default:
      case 'page_1':
        // Save, Next.
        $form[$scope]['Save'] = ['#type' => 'submit', '#value' => t('Save')];
        $form[$scope]['Next'] = [
          '#type' => 'submit',
          '#value' => t('Next'),
          '#attributes' => ['class' => ['next-button']],
        ];
        break;

      case 'page_4':
        // Back, Save, Review, Submit, Pre-validate my VCF files.
        $form[$scope]['Back'] = [
          '#type' => 'submit',
          '#value' => t('Back'),
          // Disable validation.
          // Currently useless because all fields are optional but this
          // could be changed in future. But it removed saved data of stage
          // when 'Back' button clicked which is not good.
          //'#limit_validation_errors' => [],
        ];
        $form[$scope]['Save'] = ['#type' => 'submit', '#value' => t('Save')];
        $form[$scope]['submit'] = [
          '#type' => 'submit',
          '#value' => t('Review and Submit'),
          // We add class because we have buttons above and below the form.
          '#attributes' => ['class' => ['review-and-submit-button']],
        ];

        // Pre-validate VCF.
        if ($form['form_type']['#value'] == 'tppsc') {
          $pre_valid_states = [];
          for ($i = 1; $i <= $form_bus['organism_number']; $i++) {
            $pre_valid_states["[name=\"organism-{$i}[genotype][SNPs][file-type]\"]"]
              = ['value' => TPPS_GENOTYPING_FILE_TYPE_VCF];
          }
          $form[$scope]['pre_validate'] = [
            '#type' => 'button',
            '#value' => t('Pre-validate my VCF files'),
            '#states' => ['visible' => $pre_valid_states],
            '#suffix' => '<div>Sometimes large VCF files can cause problems '
              . 'during the validation step. If your VCF file is very large, '
              . 'you can click the "Pre-validate my VCF files" button to '
              . 'validate your VCF file in the background, before clicking '
              . '"Review Information and Submit".</div>',
            '#attributes' => ['class' => ['vcf-pre-validate-button']],
          ];
          $form[$scope]['pre_validate']['#prefix'] = '<div class="pre-validate-message"></div>';
        }
        break;

      case 'summary':
        // Back, Submit.
        $form[$scope]['Back'] = [
          '#type' => 'submit',
          '#value' => t('Back'),
          // Disable validation.
          // Currently useless because all fields are optional but this
          // could be changed in future. But it removed saved data of stage
          // when 'Back' button clicked which is not good.
          //'#limit_validation_errors' => [],
        ];
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
    // drupal_set_message($message, 'error');
    return;
  }
  // Remove colon ':' and required field label.
  $title = strtok($field['#title'], ':');

  // Since we update title of the field using JS in browser we could update
  // title of the field which failed validation here.
  if ($field['#name'] == 'organism-1[genotype][other][other]') {
    $new_path = $path;
    array_pop($new_path);
    $new_path[] = 'other-marker';
    $new_name = drupal_array_get_nested_value(
      //$form_state['input'][TPPS_PAGE_4],
      $form_state['input'],
      $new_path
    );
    $title = str_replace('Other', $new_name, $title);
  }
  form_set_error(implode('][', $field['#array_parents']),
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
 * See function tpps_form_build_main() where stage of form is changed.
 *
 * @param array $form
 *   The form being populated.
 * @param array $form_state
 *   The state of the form being populated.
 */
function tpps_form_navigation_bar(array &$form, array $form_state) {
  $submission = new Submission($form_state['accession']);

  $current_stage = $submission->getCurrentStage();
  $current_stage = ($current_stage == 'summarypage' ? 5 : $current_stage);

  $last_completed_stage = $submission->getLastCompletedStage();
  $last_completed_stage = ($last_completed_stage == 'summarypage' ? 5 : $last_completed_stage);

  // @TODO Each stage has '#cleared' option which indicates that form step
  // passed validation when it's set and value is TRUE. Remove for TPPSc.
  $stage_list = [
    1 => t('Publication Information'),
    2 => t('Experimental Design'),
    3 => t('Plant Accession'),
    4 => t('Data'),
    // 'summarypage'
    5 => t('Review and Submit'),
  ];
  foreach ($stage_list as $number => $label) {
    // WARNING: Recreate $options to clean previos item settings.
    $options = ['query' => ['stage' => $number]];
    $options['attributes']['class'][] = 'stage';
    // Add CSS classes to style completed and current page links.
    if ($number <= $last_completed_stage) {
      $options['attributes']['class'][] = 'complete';
    }
    else {
      $options['attributes']['class'][] = 'incomplete';
    }
    if ($number == $current_stage) {
      $options['attributes']['class'][] = 'current';
    }
    // Plain or Active link.
    // Allow to got to next incompleted stage using link.
    if ($number > $last_completed_stage + 1) {
      $markup = ($markup ?? '')
        . '<div class="' . implode(' ', $options['attributes']['class'])
        . '">' . $label . '</div>';
    }
    else {
      $item_url = 'tppsc/' . $submission->accession;
      $markup = ($markup ?? '') . l($label, $item_url, $options);
    }
  }

  $form['navigation_bar'] = [
    '#markup' => '<div class="navigation-bar">' . $markup . '</div>',
    '#weight' => -1000,
  ];
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
 *                [2] => SNPs
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
 * @param array $form_bus
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
function tpps_form_add_yesno_field(array $form_bus) {

  // Validate required elements.
  if (empty($form_bus['form'])) {
    watchdog('tpps', 'Empty $form received', [], WATCHDOG_ERROR);
    return;
  }
  else {
    $form = &$form_bus['form'];
  }
  if (empty($form_bus['field_name']) || empty($form_bus['#title'])) {
    watchdog('tpps', 'Empty "name" or "#title" received', [], WATCHDOG_ERROR);
    return;
  }
  if (!isset($form_bus['organism_id'])) {
    return;
  }
  $field_name = $form_bus['field_name'];
  $i = $form_bus['organism_id'];

  $organism_name = 'organism-' . $i;

  // $form_state is optional and used for default value only.
  $form_state = &$form_bus['form_state'] ?? [];
  unset($form_bus['form_state']);
  // Default value.
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4] ?? NULL;
  // Options.
  // Machine names are in lower case to indicate that it's not human readable
  // name which could be localized.
  $options = ['yes' => t('Yes'), 'no' => t('No')];
  if (!($form_bus['#required'] ?? FALSE)) {
    $options = [0 => t('- Select -')] + $options;
  }
  // Build field using defaults.
  $defaults = [
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => ($form_bus['page4_values'][$field_name] ?? NULL),
    '#tree' => TRUE,
  ];
  $field = array_filter($form_bus, function ($e) {
    return strpos($e, '#') !== FALSE;
  }, ARRAY_FILTER_USE_KEY);
  $field = array_merge($defaults, $field);
  $parents = array_merge(($form_bus['parents'] ?? []), [$field_name]);
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
 * WARNING: Currently could be used only at Page 4.
 *
 * @param array $form_bus
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
 *     // Element 'extra_elements' allow to add any not expected form elements.
 *     'extra_elements' => [],
 *     'use_fid' => FALSE, // Default is FALSE. See below.
 *     'show_extensions_in_description' => TRUE, // bool. Default is FALSE.
 *     'page' => TPPS_PAGE_3; // int Default is TPPS_PAGE_4.
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
function tpps_form_build_file_field(array $form_bus) {
  if (empty($field_name = $form_bus['field_name'])) {
    return;
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Map from the $form_bus.
  $form = &$form_bus['form'];
  $form_state = $form_bus['form_state'] ?? [];
  $organism_name = $form_bus['organism_name'];
  $allow_file_reuse = $form_bus['allow_file_reuse'] ?? FALSE;
  $parents = $form_bus['parents'] ?? [];

  // @TODO Minor. Add page id to $form_bus['parents'].
  $page = $form_bus['page'] ?? TPPS_PAGE_4;
  $title = $form_bus['title'] ?? '';
  $optional = $form_bus['optional'] ?? FALSE;
  $upload_location = $form_bus['upload_location'] ?? '';
  $extensions = $form_bus['extensions'] ?? ['csv tsv'];
  $description = $form_bus['description'] ?? '';
  $states = $form_bus['states'] ?? '';
  // Some fields have used this piece of code before.
  // To use: $meta['use_fid'] = TRUE; Default is FALSE.
  $use_fid = $form_bus['use_fid'] ?? FALSE;
  $extra_elements = $form_bus['extra_elements'] ?? [];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Get treasure.
  //
  // Debug Mode.
  // When enabled field's machine name will be shown in field's decription.
  $debug_mode = FALSE;
  // Main element to work with.
  $parent_element = &drupal_array_get_nested_value($form, $parents);
  // Upload folder.
  if (empty($upload_location) && ($form_bus['type'] ?? FALSE)) {
    $upload_location = 'public://' . variable_get(
      'tpps_' . $form_bus['type'] . '_files_dir', 'tpps_' . $form_bus['type']
    );
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add checkbox to select existing file or upload new one.
  // Note: This code is outdated and not tested. Currently not used but there
  // is a task about File Reusage and this code could became useful.
  if ($allow_file_reuse) {
    $selector_name = $form_bus['field_name'] . '_file_selector';
    $parent_element[$selector_name] = [
      '#type' => 'checkbox',
      '#title' => t('Reference Existing @title', ['@title' => $form_bus['title']]),
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
    . (($form_bus['show_extensions_in_description'] ?? FALSE)
      ? t('<br/>Allowed file extensions: @list',
        ['@list' => implode(', ', $extensions)]) : ''),
    '#tree' => TRUE,
    '#states' => $states,
    //'#parents' => $parents,
  ];
  // dpm(print_r($parents, 1), 'parents of ' . $field_name);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Customization.
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
    $file = tpps_file_load($parent_element[$field_name]['#default_value'] ?? '');
    if ($file) {
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
      '#upload_location' => $upload_location,
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
            => ['!value' => TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY],
          // &&
          ':input[name="' . $organism_name . '[genotype][SNPs][file-type]"]'
            => ['!value' => TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE],
        ],
      ],
    ];
  }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Form Bus is a way to share data between functions.

/**
 * Gets submission value by short name from the $form_bus.
 *
 * @param array $form_bus
 *   Metadata. Required keys are: 'form' and 'form_state'.
 * @param string $name
 *   Short name (synonym) of the field.
 *   Possible values are:
 *     'organism_name'
 *       Organism name. Requires 3rd argument: $organism_id.
 *       Usage: tpps_form_bus_get($form_bus, 'organism_name', $orgainsm_id).
 *     'organism_number'
 *       Number of organisms in the study.
 *       Usage: tpps_form_bus_get($form_bus, 'organism_number').
 *
 * @return mixed
 *   Returns value if element not found.
 *
 * @TODO Replace with class FormBus.
 */
function &tpps_form_bus_get(array $form_bus, $name) {
  // Define NULL as variable to return it as reference.
  $null = NULL;
  // Validate.
  if (empty($form_bus) || empty($name)) {
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

    case 'organism_number':
      $parents = array_merge($page_parents[1], ['organism', 'number']);
      break;
  }

  // Get value.
  if ($parents) {
    $value = &drupal_array_get_nested_value($form_bus, $parents);
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
 * @param array $path
 *   List of parents (if any) and field name.
 *
 * @TODO Minor. Use CSS id intead of name attribute to set focus.
 */
function tpps_form_autofocus(array &$form = NULL, array $path) {
  if (empty($field = drupal_array_get_nested_value($form, $path) ?? NULL)) {
    return;
  }
  // HTML fieldname (not from field name).
  if ($field['#type'] == 'managed_file') {
    // Example: files[tree-accession_species-1_file].
    $html_name = 'files[' . implode('_', $path) . ']';
  }
  elseif (count($path) == 1) {
    // E.g., 'data-type'.
    $html_name = $path[0];
  }
  else {
    // E.g., 'organism-1[genotype][SNPs][vcf]', 'publication[status]', 'accession'.
    $html_name = str_replace($path[0] . ']', $path[0], implode('][', $path));
    $html_name .= ']';
  }
  tpps_add_css_js('autofocus', $form);
  $form['#attached']['js'][] = [
    'type' => 'setting',
    'data' => [
      'tpps' => [
        'autoFocus' => [
          'FieldName' => check_plain($html_name),
          'Timeout' => check_plain(variable_get('tpps_autofocus_delay', 1000)),
        ],
      ],
    ],
  ];
  // @TODO Minor. Field could be not on 1st level.
  unset($field['#attributes']['autofocus']);
}

/**
 * Get TPPS Form Id.
 *
 * @param array $form_state
 *   Drupal Form State array.
 *
 * @return string
 *   Returns 'tpps_main' (default) or 'tppsc_main'.
 */
function tpps_form_get_id(array $form_state) {
  $submission = new Submission();
  $submission->state = $form_state;
  return $submission->getFormId();
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
    // Can't return NULL by referrence.
    $null = NULL;
    return $null;
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
      if (empty($result['parent_element']) && tpps_is_debug_mode()) {
        tpps_message(
          'Empty parent element. @path.', ['@path' => print_r($path, 1)]
        );
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
  $submission = new Submission();
  $submission->state = $form_state;
  if ($only_number) {
    return $submission->getAccessionNumber();
  }
  else {
    return $submission->accession;
  }
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

  // Get stage from URL if possible.
  // See function tpps_form_navigation_bar() which adds navigation links.
  if (!empty($_GET['stage'])) {
    $current_stage_name = $_GET['stage'] == 5 ? 'summarypage' : (int) ($_GET['stage']);
    $submission = tpps_submission($accession)->load();
    $last_completed_stage = $submission->getLastCompletedStage();
    $last_completed_stage = ($last_completed_stage == 'summarypage' ? 5 : $last_completed_stage);
    if (
      in_array($_GET['stage'], range(1, 5))
      // Allow to go directly to next stage after last completed.
      && $_GET['stage'] <= $last_completed_stage + 1
    ) {
      $submission->state['stage'] = check_plain($current_stage_name);
      $submission->save();
    }
    // Get rid of 'stage' param in URL.
    drupal_goto($_GET['q']);
  }

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
  $form['form_type'] = [
    '#type' => 'hidden',
    '#value' => ($is_tppsc) ? 'tppsc' : 'tpps',
  ];
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
      $submission = new Submission($accession);
      $old_form_state = $submission->state;
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
        $submission = new Submission($frontpage_values['accession']);
        $old_form_state = $submission->state;
        if (!empty($old_form_state)) {
          tpps_form_state_info($form_state, $old_form_state);
        }
      }
    }

    // Hidden form accession element.
    // Note: Adding this elements to $form heare require all sub-functions to
    // use reference and not return $form array.
    $form['accession'] = [
      '#type' => 'hidden',
      '#value' => $form_state['accession'],
    ];
    $form['stage'] = [
      '#type' => 'hidden',
      '#value' => $form_state['stage'] ?? 'frontpage',
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
        $submission = new Submission($accession);
        $old_form_state = $submission->state;
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
        $submission = new Submission();
        $submission->create($form_state, $user->uid);
      }
      else {
        $submission = new Submission($frontpage_values['accession']);
      }
      $old_form_state = $submission->state;
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
  // Create a hidden form field to store  Submission Form Version.
  $form[TPPS_SUBMISSION_FORM_VERSION_KEY] = [
    '#type' => 'hidden',
    '#value' => $form_state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY] ?? 1,
    // @TODO Minor.
    // We could get verion by $accession.
    // TPPS_SUBMISSION_FORM_CURRENT_VERSION,
  ];

  // Upgrade to latest Submission Form Version.
  tpps_submission_form_version_upgrade_to_latest($form_state, $form_state);
  // Note:
  // Saving changes $form_state items and we need to get those changes back.
  $submission = tpps_submission()->setState($form_state)->save();

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Must be after tpps_init_project() which sets accession for new studies.
  $accession = $form_state['accession'] ?? $frontpage_values['accession'] ?? NULL;
  // Hidden form accession element.
  // Note: Adding this elements to $form heare require all sub-functions to
  // use reference and not return $form array.
  $form['accession'] = [
    '#type' => 'hidden',
    '#value' => $accession
  ];
  $form['stage'] = [
    '#type' => 'hidden',
    '#value' => $form_state['stage'] ?? 'frontpage',
  ];

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

    $submission->state = $form_state;
    $submission->save();

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
  $form['#attached']['js'][] = [
    'data' => [
      'tpps' => [
        'stage' => $form_state['stage'] ?? NULL,
        'accession' => $accession,
        'organismNumber' => $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'] ?? 1,
        'formType' => $form_state['tpps_type'] ?? 'tpps',
      ],
    ],
    'type' => 'setting',
  ];
  return $form;
}

/**
 * Gets list of Submission Statuses for dropdown form elements.
 *
 * @return array
 *   Returns list of statuses where keys are machine status names and
 *   values are human-readable and localized names of statuses.
 */
function tpps_form_get_submission_status_list() {
  foreach (Submission::getStatusList() as $item) {
    $output[$item] = t($item);
  }
  return $output;
}
