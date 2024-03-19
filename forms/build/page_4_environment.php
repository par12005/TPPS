<?php

/**
 * @file
 * Page 4 Environmental related functions.
 */

/**
 * Creates fields describing the environmental data for the submission.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 *
 * @return array
 *   The populated form.
 */
function tpps_environment(array &$form, array &$form_state, $id) {
  $cartogratree_env = variable_get('tpps_cartogratree_env', FALSE);

  $form[$id]['environment'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Environmental Information:</div>'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
  );

  if ($cartogratree_env and db_table_exists('cartogratree_groups') and db_table_exists('cartogratree_layers') and db_table_exists('cartogratree_fields')) {

    // @TODO Remove this extra DB request.
    // Use global $conf instead.
    // See https://api.drupal.org/api/drupal/includes%21bootstrap.inc/function/variable_get/7.x
    $query = db_select('variable', 'v')
      ->fields('v')
      ->condition('name', db_like('tpps_layer_group_') . '%', 'LIKE');

    $results = $query->execute();
    $options = array();

    while (($result = $results->fetchObject())) {
      $group_id = substr($result->name, 17);
      $group = db_select('cartogratree_groups', 'g')
        ->fields('g', array('group_id', 'group_name'))
        ->condition('group_id', $group_id)
        ->execute()
        ->fetchObject();
      $group_is_enabled = variable_get("tpps_layer_group_$group_id", FALSE);

      if ($group_is_enabled) {
        if ($group->group_name == 'WorldClim v.2 (WorldClim)') {
          $subgroups_query = db_select('cartogratree_layers', 'c')
            ->distinct()
            ->fields('c', array('subgroup_id'))
            ->condition('c.group_id', $group_id)
            ->execute();
          while (($subgroup = $subgroups_query->fetchObject())) {
            $subgroup_title = db_select('cartogratree_subgroups', 's')
              ->fields('s', array('subgroup_name'))
              ->condition('subgroup_id', $subgroup->subgroup_id)
              ->execute()
              ->fetchObject()->subgroup_name;
            $options["worldclim_subgroup_{$subgroup->subgroup_id}"] = array(
              'group_id' => $group_id,
              'group' => $group->group_name,
              'title' => $subgroup_title,
              'params' => NULL,
            );
          }
        }
        else {
          $layers_query = db_select('cartogratree_layers', 'c')
            ->fields('c', array('title', 'group_id', 'layer_id'))
            ->condition('c.group_id', $group_id);
          $layers_results = $layers_query->execute();
          while (($layer = $layers_results->fetchObject())) {
            $params_query = db_select('cartogratree_fields', 'f')
              ->fields('f', array('display_name', 'field_id'))
              ->condition('f.layer_id', $layer->layer_id);
            $params_results = $params_query->execute();
            $params = array();
            while (($param = $params_results->fetchObject())) {
              $params[$param->field_id] = $param->display_name;
            }
            $options[$layer->layer_id] = array(
              'group_id' => $layer->group_id,
              'group' => $group->group_name,
              'title' => $layer->title,
              'params' => $params,
            );
          }
        }
      }
    }

    $form[$id]['environment']['env_layers_groups'] = array(
      '#type' => 'fieldset',
      '#title' => 'CartograPlant Environmental Layer Groups: *',
      '#collapsible' => TRUE,
    );

    $form[$id]['environment']['layer_search'] = array(
      '#type' => 'textfield',
      '#title' => t('Layers Search'),
      '#description' => t('You can use this field to filter the layers in the following section.'),
    );

    $form[$id]['environment']['env_layers'] = array(
      '#type' => 'fieldset',
      '#title' => 'CartograPlant Environmental Layers: *',
      '#collapsible' => TRUE,
    );

    $form[$id]['environment']['env_params'] = array(
      '#type' => 'fieldset',
      '#title' => 'CartograPlant Environmental Layer Parameters: *',
      '#collapsible' => TRUE,
    );

    foreach ($options as $layer_id => $layer_info) {
      $layer_title = $layer_info['title'];
      $layer_group = $layer_info['group'];
      $layer_params = $layer_info['params'];

      $form[$id]['environment']['env_layers_groups'][$layer_group] = array(
        '#type' => 'checkbox',
        '#title' => $layer_group,
        '#return_value' => $layer_info['group_id'],
      );

      $form[$id]['environment']['env_layers'][$layer_title] = array(
        '#type' => 'checkbox',
        '#title' => "<strong>$layer_title</strong> - $layer_group",
        '#states' => array(
          'visible' => array(
            ':input[name="' . $id . '[environment][env_layers_groups][' . $layer_group . ']"]' => array('checked' => TRUE),
          ),
        ),
        '#return_value' => $layer_id,
      );

      if (!empty($layer_params)) {
        $form[$id]['environment']['env_params']["$layer_title"] = array(
          '#type' => 'fieldset',
          '#title' => "$layer_title Parameters",
          '#description' => t('Please select the parameters you used from the @title layer.', array('@title' => $layer_title)),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[environment][env_layers_groups][' . $layer_group . ']"]' => array('checked' => TRUE),
              ':input[name="' . $id . '[environment][env_layers][' . $layer_title . ']"]' => array('checked' => TRUE),
            ),
          ),
        );

        foreach ($layer_params as $param_id => $param) {
          $form[$id]['environment']['env_params']["$layer_title"][$param] = array(
            '#type' => 'checkbox',
            '#title' => $param,
            '#return_value' => $param_id,
          );
        }
      }
    }

    $form[$id]['environment']['env_layers']['other'] = array(
      '#type' => 'checkbox',
      '#title' => "<strong>Other custom layer</strong>",
      '#return_value' => 'other',
    );

    $form[$id]['environment']['env_layers']['other_db'] = array(
      '#type' => 'textfield',
      '#title' => t('Layer DB URL: *'),
      '#description' => t('The url of the DB providing this layer'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[environment][env_layers][other]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form[$id]['environment']['env_layers']['other_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Layer Name: *'),
      '#description' => t('The name of the layer'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[environment][env_layers][other]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form[$id]['environment']['env_layers']['other_params'] = array(
      '#type' => 'textfield',
      '#title' => t('Parameters Used: *'),
      '#description' => t('Comma-delimited list of parameters from the layer used. For example, when using parameters "rainfall" and "humidity", this field should look something like "rainfall,humidity"'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[environment][env_layers][other]"]' => array('checked' => TRUE),
        ),
      ),
    );
  }

  return $form[$id]['environment'];
}
