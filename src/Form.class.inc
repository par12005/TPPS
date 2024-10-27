<?php

namespace Drupal\tpps;

/**
 * @file
 * Contains 'Form' class.
 */

/**
 * Manages TPPS Form.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class Form {

  /**
   * Drupal Form API array which represents form.
   *
   * @var array
   */
  protected $form;

  /**
   * Drupal Form API state array. Represents submitted form values.
   *
   * @var array
   */
  protected $formState;

  /**
   * Form Id. Possible values are: 'tpps_main', 'tppsc_main'.
   *
   * @var string
   */
  protected $formId;

  /**
   * Current page key.
   *
   * Possible values are: 1..5, 'summary_page', 'welcome_page'.
   *
   * @var mixed
   */
  protected $currentPage;

  /**
   * Study accession.
   *
   * Format is: TGDRxxxx.
   *
   * @var string
   */
  protected $accession;

  /**
   * Type of the form.
   *
   * Posible values are: 'tpps', 'tppsc'.
   *
   * @var string
   */
  protected $formType;

  /**
   * Status of debug mode.
   *
   * When TRUE then debug mode is enabled and debug messages will be shown.
   *
   * @var bool
   */
  protected $debugMode = FALSE;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Class constructor.
   *
   * @param array $form
   *   Drupal Form API array.
   * @param array $form_state
   *   Drupal Form API state array.
   */
  public function __construct(array $form = [], array $form_state = []) {
    $this->setForm($form);
    $this->formState = $form_state;
    // @todo Get correct current page value.
    // $this->currentPage = $form_state...;
    // @todo Get organism number.
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
      if (property_exists($this, $name)) {
        return $this->$name;
      }
      else {
        throw new \Exception("Inexistent property: $name");
      }
    }
    //return $this;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Debug.

  /**
   * Dumps keys of the $form array.
   *
   * @param mixed $key
   *   Optional. 1st level key which must be dumped.
   */
  public function dumpFormKeys($key = NULL) {
    if (!$this->debugMode()) {
      return;
    }
    if ($key && isset($this->form[$key])) {
      dpm(
        print_r(array_keys($this->form[$key]), 1),
        t('Form keys under key "@key"', ['@key' => $key])
      );
    }
    else {
      dpm(print_r(array_keys($this->form), 1), 'Form keys');
    }
  }

  /**
   * Dumps whole $form array.
   *
   * @param mixed $key
   *   Optional. 1st level key which must be dumped.
   */
  public function dumpForm($key = NULL) {
    if (!$this->debugMode()) {
      return;
    }
    if ($key && isset($this->form[$key])) {
      dpm(
        print_r($this->form[$key], 1), t('Form key "@key"', ['@key' => $key])
      );
    }
    else {
      dpm(print_r($this->form, 1), 'Form');
    }
  }

  /**
   * Dump FormState Keys.
   */
  public function dumpFormStateKeys($key = NULL) {
    if (!$this->debugMode()) {
      return;
    }
    if ($this->accession && $this->debugMode()) {
      $url = 'api/submission/' . $this->accession . '/view/state/';
      dpm(
        url($url, ['absolute' => TRUE]),
        t('JSON Form State')
      );
    }

    if ($key && isset($this->formState[$key])) {
      dpm(
        print_r($this->formState[$key], 1),
        t('Form State key "@key"', ['@key' => $key])
      );
    }
    else {
      dpm(print_r(array_keys($this->formState), 1), 'Form State');
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Sets $form array.
   *
   * We need special setter because we also parse $form array to get useful
   * data from it.
   *
   * @param array $form
   *   Drupal Form API array.
   *
   * @return object
   *   Returns current instance to allow chaining.
   */
  public function setForm(array $form) {
    $this->form = $form;
    $this->parseFormId();
    $this->parseAccession();
    $this->parseFormType();

$this->dumpForm();

    $this->currentPage = $this->formState['stage']
      ?? $this->form['stage']['#value']
        ?? $this->formState['input']['stage']
          ?? $this->formState['values']['stage'] ?? NULL;
    return $this;
  }

  /**
   * Gets Form Id.
   *
   * @return string
   *   Returns 'tpps_main' or 'tppsc_main'.
   */
  public function &getFormId() {
    if (empty($this->formId)) {
      $this->parseFormId();
    }
    return $this->formId;
  }

  /**
   * Sets Form Id form Form State array.
   */
  public function parseFormId() {
    $this->formId
      = $this->formState['saved_values']['front_page']['form_id']
      ?? $this->formState['build_info']['form_id']
      ?? 'tpps_main';
    return $this;
  }

  /**
   * Gets form type from existing data.
   */
  public function parseFormType() {
    $this->formType = $this->form['form_type']['#value']
      ?? $this->formState['tpps_type']
      ?? $this->formState['saved_values'][TPPS_PAGE_1]['form_type']
      ?? 'tpps';
    // $this->formState['build_info']['form_id'] == "tppsc_main"
    // $this->formState['saved_values'][TPPS_PAGE_1]['form_id'] == "tppsc_main"
    return $this;
  }

  /**
   * Checks if form is designed for curation team.
   *
   * @return bool
   *   Returns TRUE if form for curation team (formId is 'tppsc_main').
   *   Returns FALSE otherwise.
   */
  public function isCuration() {
    if (empty($this->formType)) {
      $this->parseFormType();
    }
    return (bool) ($this->formType == 'tppsc');
  }

  /**
   * Creates fields for the items that have control options.
   *
   * Fields which could be added:
   * co2, humidity, light intensity, salinity, and pH.
   *
   * @param string $type
   *   Field type. The machine-readable type of control options.
   * @param string $label
   *   The human-readable label for the control options.
   */
  public function addControlFields($type, $label) {
    // The form to be updated.
    if (!isset($this->form['form']['study_info'])) {
      $this->form['form']['study_info'] = [];
    }
    $subform = &$this->form['form']['study_info'];
    $suffix = ($type == 'growth_medium' ? t('used in') : t('within'));
    $params = ['@type' => $label, '@suffix' => $suffix];
    $special_field_list = [
      'humidity',
      'light',
      'temp',
      'growth_medium',
      'ph_growth_medium',
    ];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Yes/No field.
    $field_name = $type;
    if ($type == 'treatment') {
      $title = t('Do you have information about the treatments to these plants?');
    }
    else {
      $title = t('Do you have information about the <strong>@type</strong> '
        . '@suffix the <span class="group">group</span>?', $params
      );
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // States.
    if ($type == 'co2') {
      $states = [
        'visible' => [
          ':input[name="experimental_design"]'
          => ['value' => TPPS_EXP_DESIGN_GROWTH_CHAMBER],
        ],
      ];
    }
    elseif ($type == 'treatment') {
      $states = [
        'visible' => [
          [
            ':input[name="experimental_design"]'
            => ['value' => TPPS_EXP_DESIGN_GROWTH_CHAMBER],
          ], 'or', [
            ':input[name="experimental_design"]'
            => ['value' => TPPS_EXP_DESIGN_GREENHOUSE],
          ], 'or', [
            ':input[name="experimental_design"]'
            => ['value' => TPPS_EXP_DESIGN_EXPERIMENTAL],
          ], 'or', [
            ':input[name="experimental_design"]'
            => ['value' => TPPS_EXP_DESIGN_PLANTATION],
          ],
        ],
      ];
    }
    elseif (in_array($type, $special_field_list)) {
      $states = [
        'visible' => [
          [
            ':input[name="experimental_design"]'
            => ['value' => TPPS_EXP_DESIGN_GROWTH_CHAMBER],
          ], 'or', [
            ':input[name="experimental_design"]'
            => ['value' => TPPS_EXP_DESIGN_GREENHOUSE],
          ],
        ],
      ];
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    $this->addYesNoField([
      'stage' => TPPS_PAGE_2,
      'parents' => ['study_info'],
      'field_name' => $field_name,
      '#title' => $title,
      '#default_value' => 0,
      '#required' => FALSE,
      '#states' => $states ?? [],
    ]);

    $subform[$type] = [
      '#type' => 'fieldset',
      '#tree' => TRUE,
      //'#states' => $states ?? [],
      '#states' => [
        'visible' => [
          ':input[name="study_info[' . $field_name . ']"]' => ['value' => 'yes'],
        ],
      ],
    ];

    if ($type == 'treatment') {
      $subform[$type]['header'] = [
        '#markup' => t('Which of the following treatments were applied to the plants?'),
      ];
      $treatment_options = tppsc_page2_get_treatment_options();
      foreach ($treatment_options as $option) {
        $subform[$type][$option] = [
          '#type' => 'checkbox',
          '#title' => t($option),
          '#required_when_visible' => TRUE,
          '#states' => [
            'visible' => [
              ':input[name="study_info[' . $field_name . ']"]' => ['value' => 'yes'],
            ],
          ],
        ];
        $subform[$type]["$option-description"] = [
          '#type' => 'textfield',
          '#title' => t("<strong>$option</strong> Description"),
          '#required_when_visible' => TRUE,
          '#states' => [
            'visible' => [
              ':input[name="study_info[' . $type . '][' . $option . ']"]' =>
                ['checked' => TRUE],
            ],
          ],
        ];
      }
    }
    elseif ($type == 'growth_medium') {
      $subform[$type]['rooting_type'] = [
        '#type' => 'select',
        '#title' => t('Rooting Type:'),
        '#options' => [
          0 => t('- Select -'),
          'Aeroponics' => t('Aeroponics'),
          'Hydroponics' => t('Hydroponics'),
          'Soil' => t('Soil'),
        ],
        '#required_when_visible' => TRUE,
        '#states' => [
          'visible' => [
            ':input[name="study_info[' . $field_name . ']"]' => ['value' => 'yes'],
          ],
        ],
      ];
    }
    elseif ($type == 'temp') {
      $subform[$type]['high'] = [
        '#type' => 'textfield',
        '#title' => t('Average <strong>High</strong> Temperature (in degrees Celsius):'),
        '#required_when_visible' => TRUE,
        '#states' => [
          'visible' => [
            ':input[name="study_info[' . $field_name . ']"]' => ['value' => 'yes'],
          ],
        ],
      ];
      $subform[$type]['low'] = [
        '#type' => 'textfield',
        '#required_when_visible' => TRUE,
        '#title' => t('Average <strong>Low</strong> Temperature (in degrees Celsius):'),
        '#states' => [
          'visible' => [
            ':input[name="study_info[' . $field_name . ']"]' => ['value' => 'yes'],
          ],
        ],
      ];

    }
    else {
      $title = t('Was the <strong>@label</strong> controlled or uncontrolled?',
        ['@label' => $label]
      );
      $subform[$type]['option'] = [
        '#type' => 'select',
        '#title' => $title,
        '#options' => [
          0 => t('- Select -'),
          TPPS_CONTROLLED => t('Controlled'),
          TPPS_UNCONTROLLED => t('Uncontrolled'),
        ],
      ];
      $subform[$type]['controlled'] = [
        '#type' => 'textfield',
        '#title' => t('Controlled <strong>@label</strong> Value:', ['@label' => $label]),
        '#required_when_visible' => TRUE,
        '#states' => [
          'visible' => [
            ":input[name=\"study_info[$type][option]\"]" => [
              'value' => TPPS_CONTROLLED,
            ],
          ],
        ],
      ];
      $subform[$type]['uncontrolled'] = [
        '#type' => 'textfield',
        '#title' => t('Uncontrolled <strong>@label</strong> Value:', ['@label' => $label]),
        '#required_when_visible' => TRUE,
        '#states' => [
          'visible' => [
            ":input[name=\"study_info[$type][option]\"]" => [
              'value' => TPPS_UNCONTROLLED,
            ],
          ],
        ],
      ];
    }
  }

  /**
   * Adds selectbox with 'Yes/No' options to the form.
   *
   * @param array $form_bus
   *   Associative array with metadata to build the field. Keys are:
   *
   *   Element with prefix '#' which are Drupal Form API fields.
   *   See https://api.drupal.org/api/drupal/developer%21topics%21forms_api_reference.html/7.x
   *     'field_name' string
   *        Unique name for the field. Note: it's not '#name'.
   *
   *   Required:
   *     '#title' string
   *        Localized field title.
   *   Optional: '#default_value', '#required', '#state' and etc.
   *     '#name' string
   *        Unique name in $form array and HTML 'name' attribute.
   *
   * @todo Rename $form_bus.
   * @todo Use class's properties instead of local variales.
   * @todo Get rid of watchdog calls.
   */
  public function addYesNoField(array $form_bus) {

    $form_state = &$this->formState;
    if (empty($this->form)) {
      watchdog('tpps', 'Empty $form received', [], WATCHDOG_ERROR);
      return;
    }
    else {
      $form = &$this->form;
    }

    // Validate required elements.
    if (empty($form_bus['stage'])) {
      watchdog('tpps', 'Empty stage received', [], WATCHDOG_ERROR);
      return;
    }
    else {
      $stage = $form_bus['stage'];
    }
    if (empty($form_bus['field_name']) || empty($form_bus['#title'])) {
      watchdog('tpps', 'Empty "name" or "#title" received', [], WATCHDOG_ERROR);
      return;
    }

    $field_name = $form_bus['field_name'];
    $parents = $form_bus['parents'] ?? [];
    $parents[] = $field_name;
    // Options.
    // Machine names are in lower case to indicate that it's not human readable
    // name which could be localized.
    $options = ['yes' => t('Yes'), 'no' => t('No')];
    if (!($form_bus['#required'] ?? FALSE)) {
      $options = [0 => t('- Select -')] + $options;
    }
    // Build field using defaults.
    if (!empty($form_state['saved_values'])) {
      $default_value = drupal_array_get_nested_value($form_state['saved_values'],
          array_merge([$stage], $parents)
      );
    }
    $defaults = [
      '#type' => 'select',
      '#options' => $options,
      // This default value will be overwritten by value from $form_bus.
      // Note: $form_state uses $stage but $form doesn't use it.
      '#default_value' => $default_value ?? NULL,
      '#tree' => TRUE,
    ];
    $field = array_filter($form_bus, function ($e) {
      return strpos($e, '#') !== FALSE;
    }, ARRAY_FILTER_USE_KEY);
    $field = array_merge($defaults, $field);
    drupal_array_set_nested_value($form, $parents, $field);
  }

  /**
   * Sets study accession.
   *
   * @return object
   *   Returns current instanse to allow chaining.
   */
  public function parseAccession() {
    $this->accession
      = $this->form['accession']['#value']
      ?? $this->formState['accesssion'] ?? NULL;
    return $this;
  }

  /**
   * Gets and sets debug mode status.
   *
   * @param bool $status
   *   When set then debug Mode will be set in this status.
   *   When empty then current debugMode will be returned.
   *
   * @return bool
   *   Returns current status of debug mode.
   */
  public function debugMode($status = NULL) {
    if (!is_null($status)) {
      $this->debugMode = (bool) $status;
    }
    return (bool) $this->debugMode;
  }

  /**
   * Sets focus on given field by name.
   *
   * WARNING:
   *   Be sure element has no 'autofocus' attribute because it won't work.
   *   Timeout is required to have element at page.
   * NOTE:
   *   Multistep forms could call this function multiple times for each step
   *   but if field not found in $form extra JS code will not be added to page.
   *   See js/tpps_autofocus.js file.
   *
   * @param array $path
   *   List of parents (if any) and field name.
   *
   * @todo Minor. Use CSS id intead of name attribute to set focus.
   */
  public function autoFocus(array $path) {
    if (empty($field = drupal_array_get_nested_value($this->form, $path) ?? NULL)) {
      if ($this->debugMode()) {
        dpm('Autofocus: field is missing.');
      }
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
      // E.g., 'organism-1[genotype][SNPs][vcf]',
      // 'publication[status]', 'accession'.
      $html_name = str_replace($path[0] . ']', $path[0], implode('][', $path));
      $html_name .= ']';
    }
    tpps_add_css_js('autofocus', $this->form);
    $this->form['#attached']['js'][] = [
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
    if ($this->debugMode()) {
      dpm('Autofocus: done.');
    }
    // @todo Minor. Field could be not on 1st level.
    unset($field['#attributes']['autofocus']);
  }

  /**
   * Adds buttons Back/Save/Next to the forms.
   *
   * The same buttons will be added above and below main form.
   *
   * @param string $page
   *   Page number with 'page_' prefix to improve readability.
   *
   *    @todo Get organism number from the $this->form array.
   *   'organism_number' int
   *     Organism Number.
   *   'data_type' string
   *     Study Data Type which contains:
   *       'G' for genotype,
   *       'P' for phenotype,
   *       'E' for environment.
   */
  public function addButtons($page) {
    $scope_list = ['header' => -1000, 'footer' => 1000];
    $back_button = ['#type' => 'submit', '#value' => t('Back')];

    foreach ($scope_list as $scope => $weight) {
      $this->form[$scope] = [
        '#type' => 'fieldset',
        '#tree' => FALSE,
        '#weight' => $weight,
      ];
      if ($page != 'summary') {
        // @todo [VS] Use Drupal Form API 'required' attribute.
        $this->form[$scope]['input_description'] = [
          '#markup' => '<div class="input-description">* : Required Field</div>',
          '#weight' => -($weight),
        ];
      }
      switch ($page) {
        case 'page_2':
        case 'page_3':
          // Back, Save, Next.
          $this->form[$scope]['Back'] = $back_button;

        default:
        case 'page_1':
          // Save, Next.
          $this->form[$scope]['Save'] = ['#type' => 'submit', '#value' => t('Save')];
          $this->form[$scope]['Next'] = [
            '#type' => 'submit',
            '#value' => t('Next'),
            '#attributes' => ['class' => ['next-button']],
          ];
          break;

        case 'page_4':
          // Back, Save, Review, Submit, Pre-validate my VCF files.
          $this->form[$scope]['Back'] = $back_button;
          $this->form[$scope]['Save'] = ['#type' => 'submit', '#value' => t('Save')];
          $this->form[$scope]['submit'] = [
            '#type' => 'submit',
            '#value' => t('Review and Submit'),
            // We add class because we have buttons above and below the form.
            '#attributes' => ['class' => ['review-and-submit-button']],
          ];

          // Pre-validate VCF.
          if ($this->form['form_type']['#value'] == 'tppsc') {
            // Build conditions for Drupal Form States API.
            $pre_valid_states = [];

// @TODO Get real number of organisms.


            if ($form_bus['organism_number'] == 1) {
              $pre_valid_states['[name="organism-1[genotype][SNPs][file-type]"]']
                = ['value' => TPPS_GENOTYPING_FILE_TYPE_VCF];
            }
            elseif ($form_bus['organism_number'] > 1) {
              // Multiple conditions have other format.
              $pre_valid_states[] = [
                '[name="organism-1[genotype][SNPs][file-type]"]'
                => ['value' => TPPS_GENOTYPING_FILE_TYPE_VCF],
              ];
              for ($i = 2; $i <= $form_bus['organism_number']; $i++) {
                $pre_valid_states[] = 'or';
                $pre_valid_states[] = [
                  '[name="organism-' . $i . '[genotype][SNPs][file-type]"]'
                  => ['value' => TPPS_GENOTYPING_FILE_TYPE_VCF],
                ];
              }
            }

            $this->form[$scope]['pre_validate'] = [
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
            $this->form[$scope]['pre_validate']['#prefix'] =
              '<div class="pre-validate-message"></div>';
          }
          break;

        case 'summary':
          // Back, Submit.
          $this->form[$scope]['Back'] = $back_button;
          $this->form[$scope]['Next'] = ['#type' => 'submit', '#value' => t('Submit')];
          break;
      }
    }
  }

}
