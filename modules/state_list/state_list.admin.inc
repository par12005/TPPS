<?php

/**
 * @file
 * Administrative settings form and list overview for State List module.
 *
 * @todo Move list of current lists to the /admin/reports/state_list/%.
 */

/**
 * Form builder for the State List configuration form.
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function state_list_admin_settings_form($form, &$form_state) {
  $form['storage'] = [
    '#type' => 'fieldset',
    '#title' => t('Storage Backend Settings'),
    '#description' => t('Select how lists are persisted across page requests. '
      . 'Neither option uses Drupal bootstrap variables, preventing cache '
      . 'invalidation and memory overhead.'),
  ];

  $form['storage']['state_list_storage_backend'] = [
    '#type' => 'radios',
    '#title' => t('Persistent Storage Backend'),
    '#default_value' => variable_get(
      'state_list_storage_backend',
      StateList::STORAGE_TABLE
    ),
    '#options' => [
      StateList::STORAGE_TABLE => t('<strong>Dedicated database table '
        . '({state_list})</strong> &mdash; <em>Recommended</em>. Data is '
        . 'stored in its own database table, isolated from Drupal '
        . 'bootstrap variables, and permanent across cache clears.'),
      StateList::STORAGE_CACHE => t('<strong>Drupal Cache API '
        . '({cache_state_list})</strong> &mdash; Stores lists via '
        . 'cache_get() / cache_set(). Can leverage external backends '
        . 'like Memcache or Redis.'),
    ],
  ];

  $form['storage']['state_list_cache_bin'] = [
    '#type' => 'textfield',
    '#title' => t('Cache Bin Name'),
    '#default_value' => variable_get('state_list_cache_bin', 'cache_state_list'),
    '#description' => t('The cache bin table to use when Cache API storage '
      . 'is selected. Defaults to %bin.', ['%bin' => 'cache_state_list']),
    '#states' => [
      'visible' => [
        ':input[name="state_list_storage_backend"]' => [
          'value' => StateList::STORAGE_CACHE
        ],
      ],
    ],
  ];

  // Lists Overview Fieldset.
  $form['overview'] = [
    '#type' => 'fieldset',
    '#title' => t('Active Lists Overview'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];

  $all_lists = StateList::getAllLists();
  $header = [t('List Name'), t('Item Count'), t('Type'), t('Sample Values')];
  $rows = [];

  if (!empty($all_lists)) {
    foreach ($all_lists as $name => $items) {
      $count = is_array($items) ? count($items) : 0;
      $is_assoc = is_array($items) && (array_keys($items) !== range(0, max(0, $count - 1)));
      $type = $is_assoc ? t('Key-Value (Associative)') : t('Scalar (Indexed)');

      $sample = '';
      if (!empty($items)) {
        $slice = array_slice($items, 0, 3, TRUE);
        $parts = [];
        foreach ($slice as $k => $v) {
          $val_str = is_scalar($v) ? (string) $v : gettype($v);
          $parts[] = $is_assoc ? "{$k} => {$val_str}" : $val_str;
        }
        $sample = implode(', ', $parts);
        if ($count > 3) {
          $sample .= ', &hellip;';
        }
      }

      $rows[] = [
        '<strong>' . check_plain($name) . '</strong>',
        $count,
        $type,
        $sample,
      ];
    }
  }

  $form['overview']['table'] = [
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No lists are currently stored.'),
  ];

  $form = system_settings_form($form);
  $form['#submit'][] = 'state_list_admin_settings_form_submit';
  return $form;
}

/**
 * Additional submit handler to sync StateList in-memory backend.
 */
function state_list_admin_settings_form_submit($form, &$form_state) {
  $backend = $form_state['values']['state_list_storage_backend'];
  $cache_bin = $form_state['values']['state_list_cache_bin'];
  StateList::setStorageBackend($backend, $cache_bin);
}
