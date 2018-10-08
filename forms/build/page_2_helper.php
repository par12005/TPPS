<?php


function studyDate($type, &$form, $values, &$form_state){

    $form[$type . 'Date'] = array(
      '#prefix' => "<div class='container-inline'>", 
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#suffix' => '</div>',
    );

    if ($type == "Ending"){
        $form['EndingDate']['#states'] = array(
          'invisible' => array(
            array(
              array(':input[name="StartingDate[month]"]' => array('value' => '0')),
              'or',
              array(':input[name="StartingDate[year]"]' => array('value' => '0')),
            )
          )
        );
    }
    else {
        $form[$type . 'Date']['#title'] = t('<div class="fieldset-title">Experiment Dates</div>');
    }

    $yearArr = array();
    $yearArr[0] = '- Select -';
    for ($i = 1970; $i <= 2018; $i++) {
        $yearArr[$i] = $i;
    }

    $monthArr = array(
      0 => '- Select -',
      'January' => 'January',
      'February' => 'February',
      'March' => 'March',
      'April' => 'April',
      'May' => 'May',
      'June' => 'June',
      'July' => 'July',
      'August' => 'August',
      'September' => 'September',
      'October' => 'October',
      'November' => 'November',
      'December' => 'December'
    );

    $form[$type . 'Date']['year'] = array(
      '#type' => 'select',
      '#title' => t("$type Year: *"),
      '#options' => $yearArr,
    );

    $form[$type . 'Date']['month'] = array(
      '#type' => 'select',
      '#title' => t("$type Month: *"),
      '#options' => $monthArr,
      '#states' => array(
        'invisible' => array(
          ':input[name="' . $type . 'Date[year]"]' => array('value' => '0')
        )
      ),
    );

    if ($type == "Starting"){
        $form['StartingDate']['year']['#ajax'] = array(
          'callback' => 'ajax_date_year_callback',
          'wrapper' => 'Endingyear'
        );
        $form['StartingDate']['month']['#ajax'] = array(
          'callback' => 'ajax_date_month_callback',
          'wrapper' => 'Endingmonth'
        );
    }
    else{
        $form['EndingDate']['year']['#ajax'] = array(
          'callback' => 'ajax_date_month_callback',
          'wrapper' => 'Endingmonth'
        );
        $form['EndingDate']['year']['#prefix'] = '<div id="Endingyear">';
        $form['EndingDate']['year']['#suffix'] = '</div>';
        $form['EndingDate']['month']['#prefix'] = '<div id="Endingmonth">';
        $form['EndingDate']['month']['#suffix'] = '</div>';

        if (isset($form_state['values']['StartingDate']['year']) and $form_state['values']['StartingDate']['year'] != '0'){
            $yearArr = array();
            $yearArr[0] = '- Select -';
            for ($i = $form_state['values']['StartingDate']['year']; $i <= 2018; $i++) {
                $yearArr[$i] = $i;
            }
            $form['EndingDate']['year']['#options'] = $yearArr;
        }
        if (isset($form_state['values']['EndingDate']['year']) and $form_state['values']['EndingDate']['year'] == $form_state['values']['StartingDate']['year'] and isset($form_state['values']['StartingDate']['month']) and $form_state['values']['StartingDate']['month'] != '0'){
            foreach ($monthArr as $key){
                if ($key != '0' and $key != $form_state['values']['StartingDate']['month']){
                    unset($monthArr[$key]);
                }
                elseif ($key == $form_state['values']['StartingDate']['month']){
                    break;
                }
            }
            $form['EndingDate']['month']['#options'] = $monthArr;
        }
    }

    return $form;
}

function studyLocation(&$form, $values, &$form_state){

    $form['studyLocation'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Study Location:</div>'),
      '#tree' => TRUE,
      '#collapsible' => TRUE,
    );

    $form['studyLocation']['type'] = array(
      '#type' => 'select',
      '#title' => t('Coordinate Projection: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'WGS 84',
        3 => 'NAD 83',
        4 => 'ETRS 89',
        2 => 'Custom Location (street address)'
      ),
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('left'),
        'title' => array('Please select a Coordinate Projection, or select "Custom Location", to enter a custom study location')
      )
    );

    $form['studyLocation']['coordinates'] = array(
      '#type' => 'textfield',
      '#title' => t('Coordinates: *'),
      '#states' => array(
        'visible' => array(
          array(
            array(':input[name="studyLocation[type]"]' => array('value' => '1')),
            'or',
            array(':input[name="studyLocation[type]"]' => array('value' => '3')),
            'or',
            array(':input[name="studyLocation[type]"]' => array('value' => '4')),
          )
        ),
      ),
      '#description' => 
'Accepted formats: <br>
Degrees Minutes Seconds: 41° 48\' 27.7" N, 72° 15\' 14.4" W<br>
Degrees Decimal Minutes: 41° 48.462\' N, 72° 15.24\' W<br>
Decimal Degrees: 41.8077° N, 72.2540° W<br>'
    );

    $form['studyLocation']['custom'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom Location: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="studyLocation[type]"]' => array('value' => '2')
        ),
      )
    );

    $form['studyLocation']['map-button'] = array(
      '#type' => 'button',
      '#title' => 'Click here to update map',
      '#value' => 'Click here to update map',
      '#button_type' => 'button',
      '#executes_submit_callback' => FALSE,
      '#prefix' => '<div id="page_2_map">',
      '#suffix' => '</div>',
      '#ajax' => array(
        'callback' => 'page_2_map_ajax',
        'wrapper' => 'page_2_map',
      )
    );

    if (isset($form_state['values']['studyLocation'])){
        $location = $form_state['values']['studyLocation'];
    }
    elseif (isset($form_state['saved_values']['secondPage']['studyLocation'])){
        $location = $form_state['saved_values']['secondPage']['studyLocation'];
    }

    if (isset($location)){
        if (isset($location['coordinates'])){
            $raw_coordinate = $location['coordinates'];
            $standard_coordinate = tpps_standard_coord($raw_coordinate);
        }

        if (isset($location['type']) and $location['type'] == '2' and isset($location['custom'])){
            $query = $location['custom'];
        }
        elseif (isset($location['type']) and $location['type'] != '0'){
            if ($standard_coordinate){
                $query = $standard_coordinate;
            }
            else {
                dpm('Invalid coordinates');
            }
        }

        if (isset($query) and $query != ""){
            $form['studyLocation']['map-button']['#suffix'] = "
            <br><iframe
              width=\"100%\"
              height=\"450\"
              frameborder=\"0\" style=\"border:0\"
              src=\"https://www.google.com/maps?q=$query&output=embed&key=AIzaSyDkeQ6KN6HEBxrIoiSCrCHFhIbipycqouY&z=5\" allowfullscreen>
            </iframe></div>";
        }
    }

    return $form;
}

function naturalPopulation(&$form, $values){

    $form['naturalPopulation'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Natural Population/Landscape Information:</div>'),
      '#tree' => TRUE,
      '#states' => array(
        'visible' => array(
          ':input[name="studyType"]' => array('value' => '1')
        ),
        'enabled' => array(
          ':input[name="studyType"]' => array('value' => '1')
        )
      ),
      '#collapsible' => TRUE,
    );

    $form['naturalPopulation']['season'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Seasons (select all that apply): *'),
      '#options' => drupal_map_assoc(array(
        t('Spring'),
        t('Summer'),
        t('Fall'),
        t('Winter'),
      )),
    );

    $num_arr = array();
    $num_arr[0] = '- Select -';
    for ($i = 1; $i <= 30; $i++) {
        $num_arr[$i] = $i;
    }

    $form['naturalPopulation']['assessions'] = array(
      '#type' => 'select',
      '#title' => t('Number of times the populations were assessed (on average): *'),
      '#options' => $num_arr,
    );

    return $form;
}

function growthChamber(&$form, $values){

    $form['growthChamber'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Growth Chamber Information:</div>'),
      '#tree' => TRUE,
      '#states' => array(
        'visible' => array(
          ':input[name="studyType"]' => array('value' => '2')
        ),
        'enabled' => array(
          ':input[name="studyType"]' => array('value' => '2')
        )
      ),
      '#collapsible' => TRUE,
    );

    co2($form, $values);

    humidity($form, $values);

    light($form, $values);

    $form['growthChamber']['temp'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Temperature Information:</div>'),
      '#description' => t('Please provide temperatures in Degrees Celsius'),
      '#tree' => true,
    );

    $form['growthChamber']['temp']['high'] = array(
      '#type' => 'textfield',
      '#title' => t('Average High Temperature: *'),
    );

    $form['growthChamber']['temp']['low'] = array(
      '#type' => 'textfield',
      '#title' => t('Average Low Temperature: *'),
    );

    $form['growthChamber']['rooting'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Rooting Information:</div>'),
      '#tree' => true,
    );

    $form['growthChamber']['rooting']['option'] = array(
      '#type' => 'select',
      '#title' => t('Rooting Type: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Aeroponics',
        2 => 'Hydroponics',
        3 => 'Soil',
      ),
    );

    $form['growthChamber']['rooting']['soil'] = array(
      '#type' => 'fieldset',
      '#states' => array(
        'visible' => array(
          ':input[name="growthChamber[rooting][option]"]' => array('value' => '3')
        )
      )
    );

    $form['growthChamber']['rooting']['soil']['type'] = array(
      '#type' => 'select',
      '#title' => t('Soil Type: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Sand',
        2 => 'Peat',
        3 => 'Clay',
        4 => 'Mixed',
        5 => 'Other'
      ),
    );

    $form['growthChamber']['rooting']['soil']['other'] = array(
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="growthChamber[rooting][soil][type]"]' => array('value' => '5')
        )
      )
    );

    $form['growthChamber']['rooting']['soil']['container'] = array(
      '#type' => 'textfield',
      '#title' => t('Soil Container Type: *'),
    );

    ph($form, $values);

    $treatment_options = drupal_map_assoc(array(
        t('Seasonal Environment'),
        t('Air temperature regime'),
        t('Soil Temperature regime'),
        t('Antibiotic regime'),
        t('Chemical administration'),
        t('Disease status'),
        t('Fertilizer regime'),
        t('Fungicide regime'),
        t('Gaseous regime'),
        t('Gravity Growth hormone regime'),
        t('Mechanical treatment'),
        t('Mineral nutrient regime'),
        t('Humidity regime'),
        t('Non-mineral nutrient regime'),
        t('Radiation (light, UV-B, X-ray) regime'),
        t('Rainfall regime'),
        t('Salt regime'),
        t('Watering regime'),
        t('Water temperature regime'),
        t('Pesticide regime'),
        t('pH regime'),
        t('other perturbation'),
      ));

    $form['growthChamber']['rooting']['treatment'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Treatments: *</div>'),
    );

    foreach($treatment_options as $key => $option){
        $form['growthChamber']['rooting']['treatment']["$option"] = array(
          '#type' => 'checkbox',
          '#title' => t("$option"),
        );

        $form['growthChamber']['rooting']['treatment']["$option-description"] = array(
          '#type' => 'textfield',
          '#description' => t("$option Description *"),
          '#states' => array(
            'visible' => array(
              ':input[name="growthChamber[rooting][treatment][' . $option . ']"]' => array('checked' => TRUE)
            )
          )
        );
    }

    return $form;
}

function greenhouse(&$form, $values){

    $form['greenhouse'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Greenhouse Information:</div>'),
      '#tree' => TRUE,
      '#states' => array(
        'visible' => array(
          ':input[name="studyType"]' => array('value' => '3')
        ),
        'enabled' => array(
          ':input[name="studyType"]' => array('value' => '3')
        )
      ),
      '#collapsible' => TRUE,
    );

    greenhumidity($form, $values);

    greenlight($form, $values);

    $form['greenhouse']['temp'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Temperature Information:</div>'),
      '#description' => t('Please provide temperatures in Degrees Celsius'),
      '#tree' => true,
    );

    $form['greenhouse']['temp']['high'] = array(
      '#type' => 'textfield',
      '#title' => t('Average High Temperature: *'),
    );

    $form['greenhouse']['temp']['low'] = array(
      '#type' => 'textfield',
      '#title' => t('Average Low Temperature: *'),
    );

    $form['greenhouse']['rooting'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Rooting Information:</div>'),
      '#tree' => true,
    );

    $form['greenhouse']['rooting']['option'] = array(
      '#type' => 'select',
      '#title' => t('Rooting Type: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Aeroponics',
        2 => 'Hydroponics',
        3 => 'Soil',
      ),
    );

    $form['greenhouse']['rooting']['soil'] = array(
      '#type' => 'fieldset',
      '#states' => array(
        'visible' => array(
          ':input[name="greenhouse[rooting][option]"]' => array('value' => '3')
        )
      )
    );

    $form['greenhouse']['rooting']['soil']['type'] = array(
      '#type' => 'select',
      '#title' => t('Soil Type: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Sand',
        2 => 'Peat',
        3 => 'Clay',
        4 => 'Mixed',
        5 => 'Other'
      ),
    );

    $form['greenhouse']['rooting']['soil']['other'] = array(
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="greenhouse[rooting][soil][type]"]' => array('value' => '5')
        ),
      )
    );

    $form['greenhouse']['rooting']['soil']['container'] = array(
      '#type' => 'textfield',
      '#title' => t('Soil Container Type: *'),
    );

    greenph($form, $values);

    $treatment_options = drupal_map_assoc(array(
        t('Seasonal Environment'),
        t('Air temperature regime'),
        t('Soil Temperature regime'),
        t('Antibiotic regime'),
        t('Chemical administration'),
        t('Disease status'),
        t('Fertilizer regime'),
        t('Fungicide regime'),
        t('Gaseous regime'),
        t('Gravity Growth hormone regime'),
        t('Mechanical treatment'),
        t('Mineral nutrient regime'),
        t('Humidity regime'),
        t('Non-mineral nutrient regime'),
        t('Radiation (light, UV-B, X-ray) regime'),
        t('Rainfall regime'),
        t('Salt regime'),
        t('Watering regime'),
        t('Water temperature regime'),
        t('Pesticide regime'),
        t('pH regime'),
        t('other perturbation'),
      ));

    $form['greenhouse']['rooting']['treatment'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Treatments: *</div>'),
    );

    foreach($treatment_options as $key => $option){
        $form['greenhouse']['rooting']['treatment']["$option"] = array(
          '#type' => 'checkbox',
          '#title' => t("$option"),
        );
        $form['greenhouse']['rooting']['treatment']["$option-description"] = array(
          '#type' => 'textfield',
          '#description' => t("$option Description *"),
          '#states' => array(
            'visible' => array(
              ':input[name="greenhouse[rooting][treatment][' . $option . ']"]' => array('checked' => TRUE)
            )
          )
        );
    }

    return $form;
}

function commonGarden(&$form, $values){

    $form['commonGarden'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Common Garden Information:</div>'),
      '#tree' => TRUE,
      '#states' => array(
        'visible' => array(
          ':input[name="studyType"]' => array('value' => '4')
        ),
        'enabled' => array(
          ':input[name="studyType"]' => array('value' => '4')
        )
      ),
      '#collapsible' => TRUE,
    );

    $form['commonGarden']['irrigation'] = array(
      '#type' => 'fieldset',
      '#tree' => true,
    );

    $form['commonGarden']['irrigation']['option'] = array(
      '#type' => 'select',
      '#title' => t('Irrigation Type: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Irrigation from top',
        2 => 'Irrigation from bottom',
        3 => 'Drip Irrigation',
        4 => 'Other',
        5 => 'No Irrigation',
      ),
    );

    $form['commonGarden']['irrigation']['other'] = array(
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="commonGarden[irrigation][option]"]' => array('value' => '4')
        )
      )
    );

    salinity($form, $values);

    $form['commonGarden']['bioticEnv'] = array(
      '#type' => 'fieldset',
      '#tree' => true,
    );

    $form['commonGarden']['bioticEnv']['option'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Biotic Environment: *'),
      '#options' => drupal_map_assoc(array(
        t('Herbivores'),
        t('Mutulists'),
        t('Pathogens'),
        t('Endophytes'),
        t('Other'),
        t('None'),
      )),
    );

    $form['commonGarden']['bioticEnv']['other'] = array(
      '#type' => 'textfield',
      '#title' => t('Please specify Biotic Environment Type: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="commonGarden[bioticEnv][option][Other]"]' => array('checked' => TRUE)
        )
      )
    );

    $form['commonGarden']['season'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Seasons: *'),
      '#options' => drupal_map_assoc(array(
        t('Spring'),
        t('Summer'),
        t('Fall'),
        t('Winter'),
      )),
    );

    $treatment_options = drupal_map_assoc(array(
        t('Seasonal environment'),
        t('Antibiotic regime'),
        t('Chemical administration'),
        t('Disease status'),
        t('Fertilizer regime'),
        t('Fungicide regime'),
        t('Gaseous regime'),
        t('Gravity Growth hormone regime'),
        t('Herbicide regime'),
        t('Mechanical treatment'),
        t('Mineral nutrient regime'),
        t('Non-mineral nutrient regime'),
        t('Salt regime'),
        t('Watering regime'),
        t('Pesticide regime'),
        t('pH regime'),
        t('Other perturbation')
      ));

    $form['commonGarden']['treatment'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Treatments:</div>'),
    );

    $form['commonGarden']['treatment']['check'] = array(
      '#type' => 'checkbox',
      '#title' => t('My Common Garden experiment used treatments/regimes/perturbations.'),
    );

    foreach($treatment_options as $key => $option){
        $form['commonGarden']['treatment']["$option"] = array(
          '#type' => 'checkbox',
          '#title' => t("$option"),
          '#states' => array(
            'visible' => array(
              ':input[name="commonGarden[treatment][check]"]' => array('checked' => TRUE),
            )
          )
        );
        $form['commonGarden']['treatment']["$option-description"] = array(
          '#type' => 'textfield',
          '#description' => t("$option Description *"),
          '#states' => array(
            'visible' => array(
              ':input[name="commonGarden[treatment][' . $option . ']"]' => array('checked' => TRUE),
              ':input[name="commonGarden[treatment][check]"]' => array('checked' => TRUE),
            )
          )
        );
    }

    return $form;
}

function plantation(&$form, $values){
    $form['plantation'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Plantation Information:</div>'),
      '#tree' => TRUE,
      '#states' => array(
        'visible' => array(
          ':input[name="studyType"]' => array('value' => '5')
        ),
        'enabled' => array(
          ':input[name="studyType"]' => array('value' => '5')
        )
      ),
      '#collapsible' => TRUE
    );

    $form['plantation']['season'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Seasons (select all that apply): *'),
      '#options' => drupal_map_assoc(array(
        t('Spring'),
        t('Summer'),
        t('Fall'),
        t('Winter'),
      )),
    );

    $num_arr = array();
    $num_arr[0] = '- Select -';
    for ($i = 1; $i <= 30; $i++) {
        $num_arr[$i] = $i;
    }

    $form['plantation']['assessions'] = array(
      '#type' => 'select',
      '#title' => t('Number of times the populations were assessed (on average): *'),
      '#options' => $num_arr,
    );

    $treatment_options = drupal_map_assoc(array(
        t('Seasonal environment'),
        t('Antibiotic regime'),
        t('Chemical administration'),
        t('Disease status'),
        t('Fertilizer regime'),
        t('Fungicide regime'),
        t('Gaseous regime'),
        t('Gravity Growth hormone regime'),
        t('Herbicide regime'),
        t('Mechanical treatment'),
        t('Mineral nutrient regime'),
        t('Non-mineral nutrient regime'),
        t('Salt regime'),
        t('Watering regime'),
        t('Pesticide regime'),
        t('pH regime'),
        t('Other perturbation')
      ));

    $form['plantation']['treatment'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Treatments:</div>')
    );

    $form['plantation']['treatment']['check'] = array(
      '#type' => 'checkbox',
      '#title' => t('My Plantation experiment used treatments/regimes/perturbations.'),
    );

    foreach($treatment_options as $key => $option){
        $form['plantation']['treatment']["$option"] = array(
          '#type' => 'checkbox',
          '#title' => t("$option"),
          '#states' => array(
            'visible' => array(
              ':input[name="plantation[treatment][check]"]' => array('checked' => TRUE),
            )
          )
        );
        $form['plantation']['treatment']["$option-description"] = array(
          '#type' => 'textfield',
          '#description' => t("$option Description *"),
          '#states' => array(
            'visible' => array(
              ':input[name="plantation[treatment][' . $option . ']"]' => array('checked' => TRUE),
              ':input[name="plantation[treatment][check]"]' => array('checked' => TRUE),
            )
          )
        );
    }

    return $form;
}

// growthchamber helper helpers
function co2(&$form, $values){

    $form['growthChamber']['co2Control'] = array(
      '#type' => 'fieldset',
      '#tree' => true
    );

    $form['growthChamber']['co2Control']['option'] = array(
      '#type' => 'select',
      '#title' => t('CO2 controlled or uncontrolled: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Controlled',
        2 => 'Uncontrolled'
      ),
    );

    $form['growthChamber']['co2Control']['controlled'] = array(
      '#type' => 'textfield',
      '#title' => t('Controlled CO2 Value: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="growthChamber[co2Control][option]"]' => array('value' => '1')
        )
      )
    );

    $form['growthChamber']['co2Control']['uncontrolled'] = array(
      '#type' => 'textfield',
      '#title' => t('Average CO2 Value: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="growthChamber[co2Control][option]"]' => array('value' => '2')
        )
      )
    );

    return $form;
}

function humidity(&$form, $values){

    $form['growthChamber']['humidityControl'] = array(
      '#type' => 'fieldset',
      '#tree' => true,
    );

    $form['growthChamber']['humidityControl']['option'] = array(
      '#type' => 'select',
      '#title' => t('Air Humidity controlled or uncontrolled: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Controlled',
        2 => 'Uncontrolled'
      ),
    );

    $form['growthChamber']['humidityControl']['controlled'] = array(
      '#type' => 'textfield',
      '#title' => t('Controlled Air Humidity Value: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="growthChamber[humidityControl][option]"]' => array('value' => '1')
        )
      )
    );

    $form['growthChamber']['humidityControl']['uncontrolled'] = array(
      '#type' => 'textfield',
      '#title' => t('Average Air Humidity Value: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="growthChamber[humidityControl][option]"]' => array('value' => '2')
        )
      )
    );

    return $form;
}

function light(&$form, $values){

    $form['growthChamber']['lightControl'] = array(
      '#type' => 'fieldset',
      '#tree' => true,
    );

    $form['growthChamber']['lightControl']['option'] = array(
      '#type' => 'select',
      '#title' => t('Light Intensity controlled or uncontrolled: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Controlled',
        2 => 'Uncontrolled'
      ),
    );

    $form['growthChamber']['lightControl']['controlled'] = array(
      '#type' => 'textfield',
      '#title' => t('Controlled Light Intensity Value: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="growthChamber[lightControl][option]"]' => array('value' => '1')
        )
      )
    );

    $form['growthChamber']['lightControl']['uncontrolled'] = array(
      '#type' => 'textfield',
      '#title' => t('Average Light Intensity Value: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="growthChamber[lightControl][option]"]' => array('value' => '2')
        )
      )
    );

    return $form;
}

function ph(&$form, $values){

    $form['growthChamber']['rooting']['ph'] = array(
      '#type' => 'fieldset',
      '#tree' => true
    );

    $form['growthChamber']['rooting']['ph']['option'] = array(
      '#type' => 'select',
      '#title' => t('pH controlled or uncontrolled: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Controlled',
        2 => 'Uncontrolled'
      ),
    );

    $form['growthChamber']['rooting']['ph']['controlled'] = array(
      '#type' => 'textfield',
      '#title' => t('Controlled pH Value: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="growthChamber[rooting][ph][option]"]' => array('value' => '1')
        )
      )
    );

    $form['growthChamber']['rooting']['ph']['uncontrolled'] = array(
      '#type' => 'textfield',
      '#title' => t('Average pH Value: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="growthChamber[rooting][ph][option]"]' => array('value' => '2')
        )
      )
    );

    return $form;
}

//greenhouse helper helpers
function greenhumidity(&$form, $values){

    $form['greenhouse']['humidityControl'] = array(
      '#type' => 'fieldset',
      '#tree' => true,
    );

    $form['greenhouse']['humidityControl']['option'] = array(
      '#type' => 'select',
      '#title' => t('Air Humidity controlled or uncontrolled: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Controlled',
        2 => 'Uncontrolled'
      ),
    );

    $form['greenhouse']['humidityControl']['controlled'] = array(
      '#type' => 'textfield',
      '#title' => t('Controlled Air Humidity Value: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="greenhouse[humidityControl][option]"]' => array('value' => '1')
        ),
      )
    );

    return $form;
}

function greenlight(&$form, $values){

    $form['greenhouse']['lightControl'] = array(
      '#type' => 'fieldset',
      '#tree' => true,
    );

    $form['greenhouse']['lightControl']['option'] = array(
      '#type' => 'select',
      '#title' => t('Light Intensity controlled or uncontrolled: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Controlled',
        2 => 'Uncontrolled'
      ),
    );

    $form['greenhouse']['lightControl']['controlled'] = array(
      '#type' => 'textfield',
      '#title' => t('Controlled Light Intensity Value: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="greenhouse[lightControl][option]"]' => array('value' => '1')
        )
      )
    );

    return $form;
}

function greenph(&$form, $values){

    $form['greenhouse']['rooting']['ph'] = array(
      '#type' => 'fieldset',
      '#tree' => true
    );

    $form['greenhouse']['rooting']['ph']['option'] = array(
      '#type' => 'select',
      '#title' => t('pH controlled or uncontrolled: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Controlled',
        2 => 'Uncontrolled'
      ),
    );

    $form['greenhouse']['rooting']['ph']['controlled'] = array(
      '#type' => 'textfield',
      '#title' => t('Controlled pH Value: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="greenhouse[rooting][ph][option]"]' => array('value' => '1')
        )
      )
    );

    return $form;
}

//commongarden helper helpers
function salinity(&$form, $values){

        $form['commonGarden']['salinity'] = array(
          '#type' => 'fieldset',
          '#tree' => true
        );

        $form['commonGarden']['salinity']['option'] = array(
          '#type' => 'select',
          '#title' => t('Salinity controlled or uncontrolled: *'),
          '#options' => array(
            0 => '- Select -',
            1 => 'Controlled',
            2 => 'Uncontrolled'
          ),
        );

        $form['commonGarden']['salinity']['controlled'] = array(
          '#type' => 'textfield',
          '#title' => t('Controlled Salinity Value: *'),
          '#states' => array(
            'visible' => array(
              ':input[name="commonGarden[salinity][option]"]' => array('value' => '1')
            )
          )
        );

        $form['commonGarden']['salinity']['uncontrolled'] = array(
          '#type' => 'textfield',
          '#title' => t('Average Salinity Value: *'),
          '#states' => array(
            'visible' => array(
              ':input[name="commonGarden[salinity][option]"]' => array('value' => '2')
            )
          )
        );

        return $form;
    }
