<?php

/**
 * @file
 * Helper functions use at any step of TPPS form.
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
    // @todo [VS] Use Drupal Form API 'required' attribute.
    $form[$scope]['input_description'] = [
      '#markup' => '<div class="input-description">* : Required Field</div>',
      '#weight' => -($weight),
    ];
    switch ($page) {
      case 'page_2':
      case 'page_3':
        $form[$scope]['Back'] = ['#type' => 'submit', '#value' => t('Back')];

      default:
      case 'page_1':
        $form[$scope]['Save'] = ['#type' => 'submit', '#value' => t('Save')];
        $form[$scope]['Next'] = ['#type' => 'submit', '#value' => t('Next')];
        break;

      case 'page_4':
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
            $pre_valid_states["input[name=\"organism-{$i}[genotype][files][file-type][VCF]\"]"]
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
 *   ['organism-1', 'genotype', 'files', 'file-type'].
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
 *   - ['organism-1', 'genotype', 'files', 'file-type'];
 *   - ['publication', 'doi_container', 'doi'];
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
  if (!isset($chest['i'])) {
    return;
  }
  $field_name = $chest['field_name'];
  $i = $chest['i'];

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
    '#default_value' => ($page4_values[$field_name] ?? NULL),
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
 * Moves form field to the new place and saves structure in $form_state.
 *
 * Use this function to move fields in browser for end user but
 * when submit_all.php script must be left untouched or structure of
 * $form_state must be left unchanged to avoid any data damage in database.
 *
 * Each relocated field will be wrapped with invisible container which could
 * be found in $form array by name:'relocated-field-container'.
 *
 * There is no sense to use it if "'#tree' => TRUE" wasn't used and
 * $form_state['values'] contains flat list of values.
 * Field will have unchanged parents in non-flat list of values.
 *
 * WARNING: Be sure to use $form_state but not $form for validation.
 * NOTE: $form_state['complete_form'][...][$field_name]['#parents']
 *   will have $chest['#parents'] array.
 *   and
 *   $form_state['complete_form'][...][$field_name]['#array_parents']
 *   will have $chest['new_parents'] but related to the top of the $form
 *   and  will include extra element 'relocated-field-container'
 *   which is an invisible container (not fieldset) for relocated field.
 *
 * Usage example:
 *   tpps_form_relocate_field([
 *     'form' => &$fields,
 *     'current_parents' => [],
 *     'field_name' => 'ref-genome',
 *     'new_parents' => ['SNPs'],
 *     // Optional if 'form' contains whole form:
 *     '#parents' => [$id, 'genotype'],
 *     '#name' => $i . '][genotype][ref-genome]',
 *   ]);
 *
 * @param array $chest
 *   Metadata. Keys are:
 *   'form' array
 *       Required. Drupal Form API array or it's element.
 *       Note: when form's element was used '#parents' must be specified
 *       directly because there is no way to detect correct parents.
 *
 *       WARNING: Send 'form' as a reference to get updates. E.g.,
 *       tpps_form_relocate_field(['form' => &$form, ... ]);
 *
 *   'field_name' string
 *      Required. Name of the field to be moved.
 *
 *   'current_parents' array
 *     Optional. Default is empty array.
 *     List of the parents in given $chest['form'] array.
 *     Relative to given $chest['form'].
 *     Must not include $chest['field_name'].
 *
 *   'new_parents' array
 *     Required. List of parents for a new field location.
 *     Relative to given $chest['form'].
 *     Must not include $chest['field_name'].
 *
 *   '#parents' array
 *     Form API element '#parents' for a field at new location. Those parents
 *     will be used in $form_state array.
 *     Required when $chest['form'] contains sub-element for the form.
 *     Optional if $chest['form'] contains whole form.
 *     Defaults in this case: $chest['current_parents'].
 *
 *     Could include $chest['field_name'].
 *
 *     See:
 *     https://www.drupal.org/docs/7/api/form-api/tree-and-parents ;
 *     https://api.drupal.org/api/drupal/developer%21topics%21forms_api_reference.html/7.x#parents .
 *
 *     When new location is a top level of the $form and '#parents' must
 *     be empty array then use "'#parents' => []" or "'#parents' => ['doi']".
 *     E.g.,
 *     tpps_form_relocate_field([
 *       'form' => &$form,
 *       'current_parents' => ['publication'],
 *       'field_name' => 'doi',
 *       'new_parents' => ['publication', 'test'],
 *       '#parents' => [], // or '#parents' => ['doi'],
 *       '#name' => 'doi',
 *     ]);
 *     Note: This field was previously manually relocated from $form['doi']
 *     to $form['publication']['doi'] so 'current_parents' is not [].
 *
 *   '#name' string
 *     HTML attribute name of the field. Optional when:
 *     1. $chest['form'] contains whole form, or
 *     2. Drupal Form States API not used, or
 *     3. Field's name not important and not used by JS or CSS.
 *     When not specified than will be built using $chest['current_parents'].
 */
function tpps_form_relocate_field(array $chest) {
  $form = &$chest['form'] ?? NULL;
  $field_name = $chest['field_name'] ?? NULL;
  $current_parents = $chest['current_parents'] ?? [];
  $new_parents = $chest['new_parents'] ?? NULL;

  $parents = $chest['#parents'] ?? $current_parents;
  // Add field name to the end of parents because without this $form_state
  // will not have value. We could require to add this field name in $chest
  // but it's not clear why parents must include field name,
  // not user friendly and duplicaiton of data.
  if (end($parents) != $field_name) {
    array_push($parents, $field_name);
  }
  // Validate $chest content.
  if (!$form || !$field_name || (!$new_parents && !$current_parents)) {
    return;
  }
  // Create container at new place.
  // Without this container we can't use '#parents' for the field we are moving.
  $container_key = 'relocated-field-container';
  drupal_array_set_nested_value(
    $form,
    array_merge($new_parents, [$container_key]),
    ['#type' => 'container', '#tree' => FALSE]
  );
  // Get parent form element.
  // Note: drupal_array_get_nested_value() allows to set to NULL but doesn't
  // remove index (or field name) from the form array so we need a reference
  // to the parent element to totally remove element.
  $parent_element = &drupal_array_get_nested_value($form, $current_parents);
  // Get '#name' HTML-tag attribute.
  $name = $chest['#name'] ?? NULL;
  if (empty($name)) {
    $current_parents_number = count($chest['current_parents']);
    if ($current_parents_number == 0) {
      $name = $chest['field_name'];
    }
    else {
      // Separate 1st element of array.
      $name = reset($chest['current_parents']);
      unset($chest['current_parents'][key($chest['current_parents'])]);
      // Build name for the nested field.
      $name .= (count($chest['current_parents'])
        ? '[' . implode('][', $chest['current_parents']) . ']' : '')
        . '[' . $chest['field_name'] . ']';
    }
  }
  // Relocate field to better place.
  drupal_array_set_nested_value(
    $form,
    array_merge($new_parents, [$container_key, $field_name]),
    array_merge(
      $parent_element[$field_name],
      ['#parents' => $parents, '#name' => $name]
    )
  );
  // Unset original form element.
  unset($parent_element[$field_name]);
}
