<?php

/**
 * @file
 * Gemini AI settings.
 *
 * For more details visit: https://ai.google.dev/gemini-api/docs/live.
 *
 * See classes:
 * - /src/GeminiAI.class.inc.
 */

/**
 * Gemini AI Settings Form.
 */
function tpps_gemini_settings_form(array $form, array &$form_state) {
  $form['model'] = [
    '#type' => 'fieldset',
    '#title' => t('Model'),
  ];
  $form['model']['tpps_gemini_model'] = [
    '#type' => 'select',
    '#title' => t('Model'),
    '#default_value' => variable_get('tpps_gemini_model'),
    '#options' => tpps_gemini_get_models(),
    '#description' => t('Last update 01.03.2026')
      . '<br />'
      . l(t('Rate limits'), 'https://ai.google.dev/gemini-api/docs/rate-limits'),
  ];
  $form['model']['tpps_gemini_model_info'] = [
    '#type' => 'fieldset',
    '#title' => t('Model version name patterns'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    'info' => [
      '#markup' => tpps_gemini_model_info(),
    ],
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $form['api'] = [
    '#type' => 'fieldset',
    '#title' => t('API Key'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];
  $form['api']['tpps_gemini_api_key'] = [
    '#type' => 'textfield',
    '#title' => t('Gemini API Key'),
    '#default_value' => variable_get('tpps_gemini_api_key'),
    '#description' => t(
      'To get or update API Key visit !link.',
      ['!link' => l(t('API key'), 'https://aistudio.google.com/apikey')]
    ),
  ];
  return system_settings_form($form);
}

/**
 * Gets Gemini Models as options for dropdown list.
 *
 * To update and check deprications:
 * https://ai.google.dev/gemini-api/docs/deprecations
 *
 * See also:
 * https://ai.google.dev/gemini-api/docs/rate-limits
 * https://ai.google.dev/gemini-api/docs/models
 *
 * @return array
 *   Returns associative array where key is a module name and value is a human
 *   readable description.
 *
 * @TODO Minor. Move to 'Gemini' class.
 */
function tpps_gemini_get_models() {
  return [
    'Gemini 3' => [
      'gemini-3.1-pro-preview' => t('3.1 Pro Preview [19.02.2026-No shutdown date]'),
      'gemini-3-pro-image-preview' => t('3 Pro Image Preview [20.11.2025-No shutdown date]'),
      'gemini-3-flash-preview' => t('3 Flash Preview [17.12.2025-No shutdown date]'),
      'gemini-3-pro-preview' => t('3 Pro Preview[18.11.2025-09.03.2026]'),
    ],
    'Gemini 2.5' => [
      // Pro.
      'gemini-2.5-pro' => t('2.5 Pro [17.06.2025-17.06.2026]'),
      // Flash.
      'gemini-2.5-flash' => t('2.5 Flash [17.06.2025-17.06.2026]'),
      'gemini-2.5-flash-image' => t('2.5 Flash Image [02.10.2025-02.10.2026]'),
      'gemini-2.5-flash-lite' => t('2.5 Flash Lite [July 22.07.2025-22.07.2026]'),
      'gemini-2.5-flash-lite-preview-09-25' => t('2.5 Flash Lite Preview 09-25 [25.09.2025 - No shutdown date]'),
    ],
    'Gemini 2' => [
      'gemini-2.0-flash' => t('2.0 Flash [05.02.2025-01.06.2026]'),
      'gemini-2.0-flash-001' => t('2.0 Flash 001 [05.02.2025-01.06.2026]'),
      'gemini-2.0-flash-lite' => t('2.0 Flash Lite [25.02.2025-01.06.2026]'),
      'gemini-2.0-flash-lite-001' => t('2.0 Flash Lite 001 [25.02.2025-01.06.2026]'),
    ],
  ];
}

/**
 * Builds content for the model info field.
 *
 * @return string
 *   Returns HTML string with model's info.
 */
function tpps_gemini_model_info() {
  return t("
Gemini models are available in either stable, preview, latest, or experimental versions.

<pre>
Note: The following list refers to the model string naming convention as of September, 2025. Models released prior to that may have different naming conventions. Refer to the exact model string if you are using an older model.
</pre>

<h3>Stable</h3>
Points to a specific stable model. Stable models usually don't change. Most production apps should use a specific stable model.

<pre>
For example: gemini-2.5-flash.
</pre>

<h3>Preview</h3>
Points to a preview model which may be used for production. Preview models will typically have billing enabled, might come with more restrictive rate limits and will be deprecated with at least 2 weeks notice.

<pre>
For example: gemini-2.5-flash-preview-09-2025.
</pre>

<h3>Latest</h3>
Points to the latest release for a specific model variation. This can be a stable, preview or experimental release. This alias will get hot-swapped with every new release of a specific model variation. A 2-week notice will be provided through email before the version behind latest is changed.
<pre>
For example: gemini-flash-latest.
</pre>

<h3>Experimental</h3>
<p>Points to an experimental model which will typically be not be suitable for production use and come with more restrictive rate limits. We release experimental models to gather feedback and get our latest updates into the hands of developers quickly.</p>
<p>Experimental models are not stable and availability of model endpoints is subject to change.</p>

<h3>Model deprecations</h3>
<p>For information about model deprecations, visit the !deprecations_link.</p>
",
    [
      '!deprecations_link' => l(
        t('the Gemini deprecations page'),
        'https://ai.google.dev/gemini-api/docs/deprecations'
      ),
    ]
  );
}
