<?php
function page_2_create_form(&$form, $form_state){
    
    if (isset($form_state['saved_values']['secondPage'])){
        $values = $form_state['saved_values']['secondPage'];
    }
    else{
        $values = array();
    }
    
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
          '#title' => t("$type Year:"),
          '#options' => $yearArr,
          '#default_value' => isset($values[$type . 'Date']['year']) ? $values[$type . 'Date']['year'] : 0,
        );
        
        $form[$type . 'Date']['month'] = array(
          '#type' => 'select',
          '#title' => t("$type Month:"),
          '#options' => $monthArr,
          '#default_value' => isset($values[$type . 'Date']['month']) ? $values[$type . 'Date']['month'] : 0,
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
          '#title' => t('Coordinate Projection:'),
          '#options' => array(
            0 => '- Select -',
            1 => 'WGS 84',
            3 => 'NAD 83',
            4 => 'ETRS 89',
            2 => 'Custom Location (street address)'
          ),
          '#default_value' => isset($values['studyLocation']['type']) ? $values['studyLocation']['type'] : 0,
          '#attributes' => array(
            'data-toggle' => array('tooltip'),
            'data-placement' => array('left'),
            'title' => array('Please select a Coordinate Projection, or select "Custom Location", to enter a custom study location')
          )
        );
        
        $form['studyLocation']['coordinates'] = array(
          '#type' => 'textfield',
          '#title' => t('Coordinates:'),
          '#default_value' => isset($values['studyLocation']['coordinates']) ? $values['studyLocation']['coordinates'] : NULL,
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
          '#title' => t('Custom Location:'),
          '#default_value' => isset($values['studyLocation']['custom']) ? $values['studyLocation']['custom'] : NULL,
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
          '#title' => t('Seasons (select all that apply):'),
          '#options' => drupal_map_assoc(array(
            t('Spring'),
            t('Summer'),
            t('Fall'),
            t('Winter'),
          )),
        );
        
        $form['naturalPopulation']['season']['Spring']['#default_value'] = isset($values['naturalPopulation']['season']['Spring']) ? $values['naturalPopulation']['season']['Spring'] : NULL;
        $form['naturalPopulation']['season']['Summer']['#default_value'] = isset($values['naturalPopulation']['season']['Summer']) ? $values['naturalPopulation']['season']['Summer'] : NULL;
        $form['naturalPopulation']['season']['Fall']['#default_value'] = isset($values['naturalPopulation']['season']['Fall']) ? $values['naturalPopulation']['season']['Fall'] : NULL;
        $form['naturalPopulation']['season']['Winter']['#default_value'] = isset($values['naturalPopulation']['season']['Winter']) ? $values['naturalPopulation']['season']['Winter'] : NULL;
        
        $num_arr = array();
        $num_arr[0] = '- Select -';
        for ($i = 1; $i <= 30; $i++) {
            $num_arr[$i] = $i;
        }
        
        $form['naturalPopulation']['assessions'] = array(
          '#type' => 'select',
          '#title' => t('Number of times the populations were assessed (on average):'),
          '#default_value' => isset($values['naturalPopulation']['assessions']) ? $values['naturalPopulation']['assessions'] : 0,
          '#options' => $num_arr,
        );
        
        return $form;
    }
    
    function growthChamber(&$form, $values){
        
        function co2(&$form, $values){
            
            $form['growthChamber']['co2Control'] = array(
              '#type' => 'fieldset',
              '#tree' => true
            );

            $form['growthChamber']['co2Control']['option'] = array(
              '#type' => 'select',
              '#title' => t('CO2 controlled or uncontrolled'),
              '#options' => array(
                0 => '- Select -',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => isset($values['growthChamber']['co2Control']['option']) ? $values['growthChamber']['co2Control']['option'] : 0,
            );

            $form['growthChamber']['co2Control']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled CO2 Value:'),
              '#default_value' => isset($values['growthChamber']['co2Control']['controlled']) ? $values['growthChamber']['co2Control']['controlled'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[co2Control][option]"]' => array('value' => '1')
                )
              )
            );

            $form['growthChamber']['co2Control']['uncontrolled'] = array(
              '#type' => 'textfield',
              '#title' => t('Average CO2 Value:'),
              '#default_value' => isset($values['growthChamber']['co2Control']['uncontrolled']) ? $values['growthChamber']['co2Control']['uncontrolled'] : NULL,
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
              '#title' => t('Air Humidity controlled or uncontrolled'),
              '#options' => array(
                0 => '- Select -',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => isset($values['growthChamber']['humidityControl']['option']) ? $values['growthChamber']['humidityControl']['option'] : 0,
            );

            $form['growthChamber']['humidityControl']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled Air Humidity Value:'),
              '#default_value' => isset($values['growthChamber']['humidityControl']['controlled']) ? $values['growthChamber']['humidityControl']['controlled'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[humidityControl][option]"]' => array('value' => '1')
                )
              )
            );

            $form['growthChamber']['humidityControl']['uncontrolled'] = array(
              '#type' => 'textfield',
              '#title' => t('Average Air Humidity Value:'),
              '#default_value' => isset($values['growthChamber']['humidityControl']['uncontrolled']) ? $values['growthChamber']['humidityControl']['uncontrolled'] : NULL,
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
              '#title' => t('Light Intensity controlled or uncontrolled'),
              '#options' => array(
                0 => '- Select -',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => isset($values['growthChamber']['lightControl']['option']) ? $values['growthChamber']['lightControl']['option'] : 0,
            );

            $form['growthChamber']['lightControl']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled Light Intensity Value:'),
              '#default_value' => isset($values['growthChamber']['lightControl']['controlled']) ? $values['growthChamber']['lightControl']['controlled'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[lightControl][option]"]' => array('value' => '1')
                )
              )
            );

            $form['growthChamber']['lightControl']['uncontrolled'] = array(
              '#type' => 'textfield',
              '#title' => t('Average Light Intensity Value:'),
              '#default_value' => isset($values['growthChamber']['lightControl']['uncontrolled']) ? $values['growthChamber']['lightControl']['uncontrolled'] : NULL,
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
              '#title' => t('pH controlled or uncontrolled'),
              '#options' => array(
                0 => '- Select -',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => isset($values['growthChamber']['rooting']['ph']['option']) ? $values['growthChamber']['rooting']['ph']['option'] : 0,
            );

            $form['growthChamber']['rooting']['ph']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled pH Value:'),
              '#default_value' => isset($values['growthChamber']['rooting']['ph']['controlled']) ? $values['growthChamber']['rooting']['ph']['controlled'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[rooting][ph][option]"]' => array('value' => '1')
                )
              )
            );

            $form['growthChamber']['rooting']['ph']['uncontrolled'] = array(
              '#type' => 'textfield',
              '#title' => t('Average pH Value:'),
              '#default_value' => isset($values['growthChamber']['rooting']['ph']['uncontrolled']) ? $values['growthChamber']['rooting']['ph']['uncontrolled'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[rooting][ph][option]"]' => array('value' => '2')
                )
              )
            );
            
            return $form;
        }
        
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
          '#title' => t('Average High Temperature'),
          '#default_value' => isset($values['growthChamber']['temp']['high']) ? $values['growthChamber']['temp']['high'] : NULL,
        );

        $form['growthChamber']['temp']['low'] = array(
          '#type' => 'textfield',
          '#title' => t('Average Low Temperature'),
          '#default_value' => isset($values['growthChamber']['temp']['low']) ? $values['growthChamber']['temp']['low'] : NULL,
        );
        
        $form['growthChamber']['rooting'] = array(
          '#type' => 'fieldset',
          '#title' => t('<div class="fieldset-title">Rooting Information:</div>'),
          '#tree' => true,
        );
        
        $form['growthChamber']['rooting']['option'] = array(
          '#type' => 'select',
          '#title' => t('Rooting Type:'),
          '#options' => array(
            0 => '- Select -',
            1 => 'Aeroponics',
            2 => 'Hydroponics',
            3 => 'Soil',
          ),
          '#default_value' => isset($values['growthChamber']['rooting']['option']) ? $values['growthChamber']['rooting']['option'] : 0,
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
          '#title' => t('Soil Type:'),
          '#options' => array(
            0 => '- Select -',
            1 => 'Sand',
            2 => 'Peat',
            3 => 'Clay',
            4 => 'Mixed',
            5 => 'Other'
          ),
          '#default_value' => isset($values['growthChamber']['rooting']['soil']['type']) ? $values['growthChamber']['rooting']['soil']['type'] : 0,
        );
        
        $form['growthChamber']['rooting']['soil']['other'] = array(
          '#type' => 'textfield',
          '#default_value' => isset($values['growthChamber']['rooting']['soil']['other']) ? $values['growthChamber']['rooting']['soil']['other'] : NULL,
          '#states' => array(
            'visible' => array(
              ':input[name="growthChamber[rooting][soil][type]"]' => array('value' => '5')
            )
          )
        );
        
        $form['growthChamber']['rooting']['soil']['container'] = array(
          '#type' => 'textfield',
          '#title' => t('Soil Container Type:'),
          '#default_value' => isset($values['growthChamber']['rooting']['soil']['container']) ? $values['growthChamber']['rooting']['soil']['container'] : NULL,
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
          '#title' => t('<div class="fieldset-title">Treatments:</div>'),
        );
        
        foreach($treatment_options as $key => $option){
            $form['growthChamber']['rooting']['treatment']["$option"] = array(
              '#type' => 'checkbox',
              '#title' => t("$option"),
              '#default_value' => isset($values['growthChamber']['rooting']['treatment']["$option"]) ? $values['growthChamber']['rooting']['treatment']["$option"] : NULL,
            );
            
            $form['growthChamber']['rooting']['treatment']["$option-description"] = array(
              '#type' => 'textfield',
              '#description' => t("$option Description"),
              '#default_value' => isset($values['growthChamber']['rooting']['treatment']["$option-description"]) ? $values['growthChamber']['rooting']['treatment']["$option-description"] : NULL,
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
        
        function greenhumidity(&$form, $values){
            
            $form['greenhouse']['humidityControl'] = array(
              '#type' => 'fieldset',
              '#tree' => true,
            );

            $form['greenhouse']['humidityControl']['option'] = array(
              '#type' => 'select',
              '#title' => t('Air Humidity controlled or uncontrolled'),
              '#options' => array(
                0 => '- Select -',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => isset($values['greenhouse']['humidityControl']['option']) ? $values['greenhouse']['humidityControl']['option'] : 0,
            );

            $form['greenhouse']['humidityControl']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled Air Humidity Value:'),
              '#default_value' => isset($values['greenhouse']['humidityControl']['controlled']) ? $values['greenhouse']['humidityControl']['controlled'] : NULL,
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
              '#title' => t('Light Intensity controlled or uncontrolled'),
              '#options' => array(
                0 => '- Select -',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => isset($values['greenhouse']['lightControl']['option']) ? $values['greenhouse']['lightControl']['option'] : 0,
            );

            $form['greenhouse']['lightControl']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled Light Intensity Value:'),
              '#default_value' => isset($values['greenhouse']['lightControl']['controlled']) ? $values['greenhouse']['lightControl']['controlled'] : NULL,
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
              '#title' => t('pH controlled or uncontrolled'),
              '#options' => array(
                0 => '- Select -',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => isset($values['greenhouse']['rooting']['ph']['option']) ? $values['greenhouse']['rooting']['ph']['option'] : 0,
            );

            $form['greenhouse']['rooting']['ph']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled pH Value:'),
              '#default_value' => isset($values['greenhouse']['rooting']['ph']['controlled']) ? $values['greenhouse']['rooting']['ph']['controlled'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="greenhouse[rooting][ph][option]"]' => array('value' => '1')
                )
              )
            );
            
            return $form;
        }
        
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
          '#title' => t('Average High Temperature:'),
          '#default_value' => isset($values['greenhouse']['temp']['high']) ? $values['greenhouse']['temp']['high'] : NULL,
        );

        $form['greenhouse']['temp']['low'] = array(
          '#type' => 'textfield',
          '#title' => t('Average Low Temperature:'),
          '#default_value' => isset($values['greenhouse']['temp']['low']) ? $values['greenhouse']['temp']['low'] : NULL,
        );
        
        $form['greenhouse']['rooting'] = array(
          '#type' => 'fieldset',
          '#title' => t('<div class="fieldset-title">Rooting Information:</div>'),
          '#tree' => true,
        );
        
        $form['greenhouse']['rooting']['option'] = array(
          '#type' => 'select',
          '#title' => t('Rooting Type:'),
          '#options' => array(
            0 => '- Select -',
            1 => 'Aeroponics',
            2 => 'Hydroponics',
            3 => 'Soil',
          ),
          '#default_value' => isset($values['greenhouse']['rooting']['option']) ? $values['greenhouse']['rooting']['option'] : 0,
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
          '#title' => t('Soil Type:'),
          '#options' => array(
            0 => '- Select -',
            1 => 'Sand',
            2 => 'Peat',
            3 => 'Clay',
            4 => 'Mixed',
            5 => 'Other'
          ),
          '#default_value' => isset($values['greenhouse']['rooting']['soil']['type']) ? $values['greenhouse']['rooting']['soil']['type'] : 0,
        );
        
        $form['greenhouse']['rooting']['soil']['other'] = array(
          '#type' => 'textfield',
          '#default_value' => isset($values['greenhouse']['rooting']['soil']['other']) ? $values['greenhouse']['rooting']['soil']['other'] : NULL,
          '#states' => array(
            'visible' => array(
              ':input[name="greenhouse[rooting][soil][type]"]' => array('value' => '5')
            ),
          )
        );
        
        $form['greenhouse']['rooting']['soil']['container'] = array(
          '#type' => 'textfield',
          '#title' => t('Soil Container Type:'),
          '#default_value' => isset($values['greenhouse']['rooting']['soil']['container']) ? $values['greenhouse']['rooting']['soil']['container'] : NULL,
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
          '#title' => t('<div class="fieldset-title">Treatments:</div>'),
        );
        
        foreach($treatment_options as $key => $option){
            $form['greenhouse']['rooting']['treatment']["$option"] = array(
              '#type' => 'checkbox',
              '#title' => t("$option"),
              '#default_value' => isset($values['greenhouse']['rooting']['treatment']["$option"]) ? $values['greenhouse']['rooting']['treatment']["$option"] : NULL,
            );
            $form['greenhouse']['rooting']['treatment']["$option-description"] = array(
              '#type' => 'textfield',
              '#description' => t("$option Description"),
              '#default_value' => isset($values['greenhouse']['rooting']['treatment']["$option-description"]) ? $values['greenhouse']['rooting']['treatment']["$option-description"] : NULL,
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
        
        function salinity(&$form, $values){
            
            $form['commonGarden']['salinity'] = array(
              '#type' => 'fieldset',
              '#tree' => true
            );

            $form['commonGarden']['salinity']['option'] = array(
              '#type' => 'select',
              '#title' => t('Salinity controlled or uncontrolled'),
              '#options' => array(
                0 => '- Select -',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => isset($values['commonGarden']['salinity']['option']) ? $values['commonGarden']['salinity']['option'] : 0,
            );

            $form['commonGarden']['salinity']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled Salinity Value:'),
              '#default_value' => isset($values['commonGarden']['salinity']['controlled']) ? $values['commonGarden']['salinity']['controlled'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="commonGarden[salinity][option]"]' => array('value' => '1')
                )
              )
            );

            $form['commonGarden']['salinity']['uncontrolled'] = array(
              '#type' => 'textfield',
              '#title' => t('Average Salinity Value:'),
              '#default_value' => isset($values['commonGarden']['salinity']['uncontrolled']) ? $values['commonGarden']['salinity']['uncontrolled'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="commonGarden[salinity][option]"]' => array('value' => '2')
                )
              )
            );
            
            return $form;
        }
        
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
          '#title' => t('Irrigation Type:'),
          '#options' => array(
            0 => '- Select -',
            1 => 'Irrigation from top',
            2 => 'Irrigation from bottom',
            3 => 'Drip Irrigation',
            4 => 'Other',
            5 => 'No Irrigation',
          ),
          '#default_value' => isset($values['commonGarden']['irrigation']['option']) ? $values['commonGarden']['irrigation']['option'] : 0,
        );
        
        $form['commonGarden']['irrigation']['other'] = array(
          '#type' => 'textfield',
          '#default_value' => isset($values['commonGarden']['irrigation']['other']) ? $values['commonGarden']['irrigation']['other'] : NULL,
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
          '#title' => t('Biotic Environment:'),
          '#options' => drupal_map_assoc(array(
            t('Herbivores'),
            t('Mutulists'),
            t('Pathogens'),
            t('Endophytes'),
            t('Other'),
            t('None'),
          )),
        );
        
        $form['commonGarden']['bioticEnv']['option']['Herbivores']['#default_value'] = isset($values['commonGarden']['bioticEnv']['option']['Herbivores']) ? $values['commonGarden']['bioticEnv']['option']['Herbivores'] : NULL;
        $form['commonGarden']['bioticEnv']['option']['Mutilists']['#default_value'] = isset($values['commonGarden']['bioticEnv']['option']['Mutilists']) ? $values['commonGarden']['bioticEnv']['option']['Mutilists'] : NULL;
        $form['commonGarden']['bioticEnv']['option']['Pathogens']['#default_value'] = isset($values['commonGarden']['bioticEnv']['option']['Pathogens']) ? $values['commonGarden']['bioticEnv']['option']['Pathogens'] : NULL;
        $form['commonGarden']['bioticEnv']['option']['Endophytes']['#default_value'] = isset($values['commonGarden']['bioticEnv']['option']['Endophytes']) ? $values['commonGarden']['bioticEnv']['option']['Endophytes'] : NULL;
        $form['commonGarden']['bioticEnv']['option']['Other']['#default_value'] = isset($values['commonGarden']['bioticEnv']['option']['Other']) ? $values['commonGarden']['bioticEnv']['option']['Other'] : NULL;
        $form['commonGarden']['bioticEnv']['option']['None']['#default_value'] = isset($values['commonGarden']['bioticEnv']['option']['None']) ? $values['commonGarden']['bioticEnv']['option']['None'] : NULL;
        
        $form['commonGarden']['bioticEnv']['other'] = array(
          '#type' => 'textfield',
          '#title' => t('Please specify Biotic Environment Type:'),
          '#default_value' => isset($values['commonGarden']['bioticEnv']['other']) ? $values['commonGarden']['bioticEnv']['other'] : NULL,
          '#states' => array(
            'visible' => array(
              ':input[name="commonGarden[bioticEnv][option][Other]"]' => array('checked' => TRUE)
            )
          )
        );
        
        $form['commonGarden']['season'] = array(
          '#type' => 'checkboxes',
          '#title' => t('Seasons:'),
          '#options' => drupal_map_assoc(array(
            t('Spring'),
            t('Summer'),
            t('Fall'),
            t('Winter'),
          )),
        );
        
        $form['commonGarden']['season']['Spring']['#default_value'] = isset($values['commonGarden']['season']['Spring']) ? $values['commonGarden']['season']['Spring'] : NULL;
        $form['commonGarden']['season']['Summer']['#default_value'] = isset($values['commonGarden']['season']['Summer']) ? $values['commonGarden']['season']['Summer'] : NULL;
        $form['commonGarden']['season']['Fall']['#default_value'] = isset($values['commonGarden']['season']['Fall']) ? $values['commonGarden']['season']['Fall'] : NULL;
        $form['commonGarden']['season']['Winter']['#default_value'] = isset($values['commonGarden']['season']['Winter']) ? $values['commonGarden']['season']['Winter'] : NULL;
        
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
          '#default_value' => isset($values['commonGarden']['treatment']['check']) ? $values['commonGarden']['treatment']['check'] : NULL,
        );
        
        foreach($treatment_options as $key => $option){
            $form['commonGarden']['treatment']["$option"] = array(
              '#type' => 'checkbox',
              '#title' => t("$option"),
              '#default_value' => isset($values['commonGarden']['treatment']["$option"]) ? $values['commonGarden']['treatment']["$option"] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="commonGarden[treatment][check]"]' => array('checked' => TRUE),
                )
              )
            );
            $form['commonGarden']['treatment']["$option-description"] = array(
              '#type' => 'textfield',
              '#description' => t("$option Description"),
              '#default_value' => isset($values['commonGarden']['treatment']["$option-description"]) ? $values['commonGarden']['treatment']["$option-description"] : NULL,
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
          '#title' => t('Seasons (select all that apply):'),
          '#options' => drupal_map_assoc(array(
            t('Spring'),
            t('Summer'),
            t('Fall'),
            t('Winter'),
          )),
        );
        
        $form['plantation']['season']['Spring']['#default_value'] = isset($values['plantation']['season']['Spring']) ? $values['plantation']['season']['Spring'] : NULL;
        $form['plantation']['season']['Summer']['#default_value'] = isset($values['plantation']['season']['Summer']) ? $values['plantation']['season']['Summer'] : NULL;
        $form['plantation']['season']['Fall']['#default_value'] = isset($values['plantation']['season']['Fall']) ? $values['plantation']['season']['Fall'] : NULL;
        $form['plantation']['season']['Winter']['#default_value'] = isset($values['plantation']['season']['Winter']) ? $values['plantation']['season']['Winter'] : NULL;
        
        $num_arr = array();
        $num_arr[0] = '- Select -';
        for ($i = 1; $i <= 30; $i++) {
            $num_arr[$i] = $i;
        }
        
        $form['plantation']['assessions'] = array(
          '#type' => 'select',
          '#title' => t('Number of times the populations were assessed (on average):'),
          '#default_value' => isset($values['plantation']['assessions']) ? $values['plantation']['assessions'] : 0,
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
          '#default_value' => isset($values['plantation']['treatment']['check']) ? $values['plantation']['treatment']['check'] : NULL,
        );
        
        foreach($treatment_options as $key => $option){
            $form['plantation']['treatment']["$option"] = array(
              '#type' => 'checkbox',
              '#title' => t("$option"),
              '#default_value' => isset($values['plantation']['treatment']["$option"]) ? $values['plantation']['treatment']["$option"] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="plantation[treatment][check]"]' => array('checked' => TRUE),
                )
              )
            );
            $form['plantation']['treatment']["$option-description"] = array(
              '#type' => 'textfield',
              '#description' => t("$option Description"),
              '#default_value' => isset($values['plantation']['treatment']["$option-description"]) ? $values['plantation']['treatment']["$option-description"] : NULL,
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
    
    studyDate('Starting', $form, $values, $form_state);
    
    studyDate('Ending', $form, $values, $form_state);
    
    studyLocation($form, $values, $form_state);
    
    $form['dataType'] = array(
      '#type' => 'select',
      '#title' => t('Data Type:'),
      '#options' => array(
        '- Select -',
        'Genotype x Phenotype',
        'Genotype',
        'Genotype x Phenotype x Environment',
        'Phenotype x Environment',
        'Genotype x Environment'
      ),
      '#default_value' => isset($values['dataType']) ? $values['dataType'] : 0,
    );

    $form['studyType'] = array(
      '#type' => 'select',
      '#title' => t('Study Type:'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Natural Population (Landscape)',
        2 => 'Growth Chamber',
        3 => 'Greenhouse',
        4 => 'Experimental/Common Garden',
        5 => 'Plantation',
      ),
      '#default_value' => isset($values['studyType']) ? $values['studyType'] : 0,
    );
    
    naturalPopulation($form, $values);
    
    growthChamber($form, $values);
    
    greenhouse($form, $values);
    
    commonGarden($form, $values);
    
    plantation($form, $values);
    
    $form['Back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
    );
    
    $form['Save'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
    
    $form['Next'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
    );

    //drupal_add_js(drupal_get_path('module', 'custom_module') . "/custom_module.js");

    return $form;
}

function ajax_date_year_callback(&$form, $form_state){
    
    return $form['EndingDate']['year'];
}

function ajax_date_month_callback(&$form, $form_state){
    
    return $form['EndingDate']['month'];
}

function page_2_map_ajax($form, $form_state){
    return $form['studyLocation']['map-button'];
}

function page_2_validate_form(&$form, &$form_state){
    if ($form_state['submitted'] == '1'){
        $form_values = $form_state['values'];
        $start_date = $form_values['StartingDate'];
        $end_date = $form_values['EndingDate'];
        $location_type = $form_values['studyLocation']['type'];
        $coordinates = $form_values['studyLocation']['coordinates'];
        $custom_location = $form_values['studyLocation']['custom'];
        $data_type = $form_values['dataType'];
        $study_type = $form_values['studyType'];
        $natural_population = $form_values['naturalPopulation'];
        $growth_chamber = $form_values['growthChamber'];
        $greenhouse = $form_values['greenhouse'];
        $common_garden = $form_values['commonGarden'];
        $plantation = $form_values['plantation'];

        if ($start_date['year'] == '0'){
            form_set_error('StartingDate][year', 'Year: field is required.');
        }
        elseif($start_date['month'] == '0'){
            form_set_error('StartingDate][month', 'Month: field is required.');
        }

        if ($end_date['year'] == '0'){
            form_set_error('EndingDate][year', 'Year: field is required.');
        }
        elseif($end_date['month'] == '0'){
            form_set_error('EndingDate][month', 'Month: field is required.');
        }

        if ($location_type == '0'){
            form_set_error('studyLocation][type', 'Location Format: field is required.');
        }
        elseif ($location_type == '1' or $location_type == '3' or $location_type == '4'){
            if ($coordinates == ''){
                form_set_error('studyLocation][coordinates', 'Coordinates: field is required.');
            }
        }
        else{
            if($custom_location == ''){
                form_set_error('studyLocation][custom', 'Custom Location: field is required.');
            }
        }

        if ($data_type == '0'){
            form_set_error('dataType', 'Data Type: field is required.');
        }

        switch ($study_type){
            case '0':
                form_set_error('studyType', 'Study Type: field is required.');
                break;
            case '1':
                //natural population
                $seasons = $natural_population['season'];
                $seasons_check = ($seasons['Spring'] . $seasons['Summer'] . $seasons['Fall'] . $seasons['Winter']);
                if ($seasons_check == '0000'){
                    form_set_error('naturalPopulation][season', 'Seasons: field is required.');
                }

                $assessions = $natural_population['assessions'];
                if ($assessions == '0'){
                    form_set_error('naturalPopulation][assessions', 'Number of times the populations were assessed: field is required.');
                }
                break;
            case '2':
                //growth chamber
                $temp_high = $growth_chamber['temp']['high'];
                $temp_low = $growth_chamber['temp']['low'];

                function valid_co2($growth_chamber){
                    $co2_control = $growth_chamber['co2Control']['option'];
                    $co2_controlled = $growth_chamber['co2Control']['controlled'];
                    $co2_uncontrolled = $growth_chamber['co2Control']['uncontrolled'];
                    if ($co2_control == '0'){
                        form_set_error('growthChamber][co2Control][option', 'CO2 controlled or uncontrolled: field is required.');
                    }
                    elseif ($co2_control == '1' and $co2_controlled == ''){
                        form_set_error('growthChamber][co2Control][controlled', 'Controlled CO2 Value: field is required.');
                    }
                    elseif ($co2_control == '2' and $co2_uncontrolled == ''){
                        form_set_error('growthChamber][co2Control][uncontrolled', 'Average CO2 Value: field is required.');
                    }
                }

                function valid_humidity($growth_chamber){
                    $humidity_control = $growth_chamber['humidityControl']['option'];
                    $humidity_controlled = $growth_chamber['humidityControl']['controlled'];
                    $humidity_uncontrolled = $growth_chamber['humidityControl']['uncontrolled'];
                    if ($humidity_control == '0'){
                        form_set_error('growthChamber][humidityControl][option', 'Air Humidity controlled or uncontrolled: field is required.');
                    }
                    elseif ($humidity_control == '1' and $humidity_controlled == ''){
                        form_set_error('growthChamber][humidityControl][controlled', 'Controlled Air Humidity Value: field is required.');
                    }
                    elseif ($humidity_control == '2' and $humidity_uncontrolled == ''){
                        form_set_error('growthChamber][humidityControl][uncontrolled', 'Average Air Humidity Value: field is required.');
                    }
                }

                function valid_light($growth_chamber){
                    $light_control = $growth_chamber['lightControl']['option'];
                    $light_controlled = $growth_chamber['lightControl']['controlled'];
                    $light_uncontrolled = $growth_chamber['lightControl']['uncontrolled'];
                    if ($light_control == '0'){
                        form_set_error('growthChamber][lightControl][option', 'Light Intensity controlled or uncontrolled: field is required.');
                    }
                    elseif ($light_control == '1' and $light_controlled == ''){
                        form_set_error('growthChamber][lightControl][controlled', 'Controlled Light Intensity Value: field is required.');
                    }
                    elseif ($light_control == '2' and $light_uncontrolled == ''){
                        form_set_error('growthChamber][lightControl][uncontrolled', 'Average Light Intensity Value: field is required.');
                    }
                }

                function valid_ph($growth_chamber){
                    $ph_control = $growth_chamber['rooting']['ph']['option'];
                    $ph_controlled = $growth_chamber['rooting']['ph']['controlled'];
                    $ph_uncontrolled = $growth_chamber['rooting']['ph']['uncontrolled'];
                    if ($ph_control == '0'){
                        form_set_error('growthChamber][rooting][ph][option', 'pH controlled or uncontrolled: field is required.');
                    }
                    elseif ($ph_control == '1' and $ph_controlled == ''){
                        form_set_error('growthChamber][rooting][ph][controlled', 'Controlled pH Value: field is required.');
                    }
                    elseif ($ph_control == '2' and $ph_uncontrolled == ''){
                        form_set_error('growthChamber][rooting][ph][uncontrolled', 'Average pH Value: field is required.');
                    }
                }

                function valid_rooting($growth_chamber){
                    $rooting_option = $growth_chamber['rooting']['option'];
                    $soil_type = $growth_chamber['rooting']['soil']['type'];
                    $soil_container = $growth_chamber['rooting']['soil']['container'];
                    $soil_other = $growth_chamber['rooting']['soil']['other'];
                    $treatment = $growth_chamber['rooting']['treatment'];

                    if ($rooting_option == '0'){
                        form_set_error('growthChamber][rooting][option', 'Rooting Type: field is required.');
                    }
                    elseif($rooting_option == '3'){
                        if ($soil_type == '0'){
                            form_set_error('growthChamber][rooting][soil][type', 'Soil Type: field is required.');
                        }
                        elseif($soil_type == '5' and $soil_other == ''){
                            form_set_error('growthChamber][rooting][soil][other', 'Custom Soil Type: field is required.');
                        }

                        if($soil_container == ''){
                            form_set_error('growthChamber][rooting][soil][container', 'Soil Container Type: field is required.');
                        }
                    }

                    valid_ph($growth_chamber);

                    //consider using some counter variable to take every other item in $treatment. 
                    $selected = false;
                    $description = false;

                    foreach ($treatment as $field => $value){
                        if (!$description){
                            $description = true;
                            $selected = $value;
                            continue;
                        }
                        elseif ($selected == '1' and $value == ''){
                            form_set_error("growthChamber][rooting][treatment][$field", "$field: field is required.");
                        }
                        $description = false;
                    }
                }

                valid_co2($growth_chamber);

                valid_humidity($growth_chamber);

                valid_light($growth_chamber);

                if ($temp_high == ''){
                    form_set_error('growthChamber][temp][high', 'Average High Temperature: field is required.');
                }

                if ($temp_low == ''){
                    form_set_error('growthChamber][temp][low', 'Average Low Temperature: field is required.');
                }

                valid_rooting($growth_chamber);

                break;
            case '3':
                //greenhouse
                $green_temp_high = $greenhouse['temp']['high'];
                $green_temp_low = $greenhouse['temp']['low'];

                function green_valid_humidity($greenhouse){
                    $humidity_control = $greenhouse['humidityControl']['option'];
                    $humidity_controlled = $greenhouse['humidityControl']['controlled'];
                    if ($humidity_control == '0'){
                        form_set_error('greenhouse][humidityControl][option', 'Air Humidity controlled or uncontrolled: field is required.');
                    }
                    elseif ($humidity_control == '1' and $humidity_controlled == ''){
                        form_set_error('greenhouse][humidityControl][controlled', 'Controlled Air Humidity Value: field is required.');
                    }
                }

                function green_valid_light($greenhouse){
                    $light_control = $greenhouse['lightControl']['option'];
                    $light_controlled = $greenhouse['lightControl']['controlled'];
                    if ($light_control == '0'){
                        form_set_error('greenhouse][lightControl][option', 'Light Intensity controlled or uncontrolled: field is required.');
                    }
                    elseif ($light_control == '1' and $light_controlled == ''){
                        form_set_error('greenhouse][lightControl][controlled', 'Controlled Light Intensity Value: field is required.');
                    }
                }

                function green_valid_ph($greenhouse){
                    $ph_control = $greenhouse['rooting']['ph']['option'];
                    $ph_controlled = $greenhouse['rooting']['ph']['controlled'];
                    if ($ph_control == '0'){
                        form_set_error('greenhouse][rooting][ph][option', 'pH controlled or uncontrolled: field is required.');
                    }
                    elseif ($ph_control == '1' and $ph_controlled == ''){
                        form_set_error('greenhouse][rooting][ph][controlled', 'Controlled pH Value: field is required.');
                    }
                }

                function green_valid_rooting($greenhouse){
                    $rooting_option = $greenhouse['rooting']['option'];
                    $soil_type = $greenhouse['rooting']['soil']['type'];
                    $soil_container = $greenhouse['rooting']['soil']['container'];
                    $soil_other = $greenhouse['rooting']['soil']['other'];
                    $treatment = $greenhouse['rooting']['treatment'];

                    if ($rooting_option == '0'){
                        form_set_error('greenhouse][rooting][option', 'Rooting Type: field is required.');
                    }
                    elseif($rooting_option == '3'){
                        if ($soil_type == '0'){
                            form_set_error('greenhouse][rooting][soil][type', 'Soil Type: field is required.');
                        }
                        elseif($soil_type == '5' and $soil_other == ''){
                            form_set_error('greenhouse][rooting][soil][other', 'Custom Soil Type: field is required.');
                        }

                        if($soil_container == ''){
                            form_set_error('greenhouse][rooting][soil][container', 'Soil Container Type: field is required.');
                        }
                    }

                    green_valid_ph($greenhouse);

                    //consider using some counter variable to take every other item in $treatment. 
                    $selected = false;
                    $description = false;

                    foreach ($treatment as $field => $value){
                        if (!$description){
                            $description = true;
                            $selected = $value;
                            continue;
                        }
                        elseif ($selected == '1' and $value == ''){
                            form_set_error("greenhouse][rooting][treatment][$field", "$field: field is required.");
                        }
                        $description = false;
                    }
                }

                green_valid_humidity($greenhouse);

                green_valid_light($greenhouse);

                if ($green_temp_high == ''){
                    form_set_error('greenhouse][temp][high', 'Average High Temperature: field is required.');
                }

                if ($green_temp_low == ''){
                    form_set_error('greenhouse][temp][low', 'Average Low Temperature: field is required.');
                }

                green_valid_rooting($greenhouse);

                break;
            case '4':
                //common garden
                $irrigation_option = $common_garden['irrigation']['option'];
                $custom_irrigation = $common_garden['irrigation']['other'];
                $biotic_env = $common_garden['bioticEnv']['option'];
                $biotic_env_check = ($biotic_env['Herbivores'] . $biotic_env['Mutulists'] . $biotic_env['Pathogens'] . $biotic_env['Endophytes'] . $biotic_env['Other'] . $biotic_env['None']);
                $custom_biotic_env = $common_garden['bioticEnv']['other'];
                $seasons = $common_garden['season'];
                $seasons_check = ($seasons['Spring'] . $seasons['Summer'] . $seasons['Fall'] . $seasons['Winter']);
                $treatment = $common_garden['treatment'];

                function valid_salinity($common_garden){
                    $salinity_control = $common_garden['salinity']['option'];
                    $salinity_controlled = $common_garden['salinity']['controlled'];
                    $salinity_uncontrolled = $common_garden['salinity']['uncontrolled'];
                    if ($salinity_control == '0'){
                        form_set_error('commonGarden][salinity][option', 'Salinity controlled or uncontrolled: field is required.');
                    }
                    elseif ($salinity_control == '1' and $salinity_controlled == ''){
                        form_set_error('commonGarden][salinity][controlled', 'Controlled Salinity Value: field is required.');
                    }
                    elseif ($salinity_control == '2' and $salinity_uncontrolled == ''){
                        form_set_error('commonGarden][salinity][uncontrolled', 'Average Salinity Value: field is required.');
                    }
                }

                if ($irrigation_option == '0'){
                    form_set_error('commonGarden][irrigation][option', 'Irrigation Type: field is required.');
                }
                elseif($irrigation_option == '4' and $custom_irrigation == ''){
                    form_set_error('commonGarden][irrigation][other', 'Custom Irrigation Type: field is required.');
                }

                valid_salinity($common_garden);

                if ($biotic_env_check == '000000'){
                    form_set_error('commonGarden][bioticEnv][option', 'Biotic Environment: field is required.');
                }
                elseif($biotic_env['Other'] != '0' and $custom_biotic_env == ''){
                    form_set_error('commonGarden][bioticEnv][other', 'Custom Biotic Environment: field is required.');
                }

                if ($seasons_check == '0000'){
                    form_set_error('commonGarden][season', 'Seasons: field is required.');
                }

                if ($treatment['check'] == '1'){
                    $selected = false;
                    $description = false;
                    $treatment_empty = true;

                    foreach ($treatment as $field => $value){
                        if ($field != 'check'){
                            if (!$description){
                                $description = true;
                                $selected = $value;
                                if ($value == '1'){
                                    $treatment_empty = false;
                                }
                                continue;
                            }
                            elseif ($selected == '1' and $value == ''){
                                form_set_error("commonGarden][treatment][$field", "$field: field is required.");
                            }
                            $description = false;
                        }
                    }

                    if ($treatment_empty){
                        form_set_error("commonGarden][treatment", 'Treatment: field is required.');
                    }
                }

                break;
            case '5':
                //Plantation
                $seasons = $plantation['season'];
                $treatment = $plantation['treatment'];
                $seasons_check = ($seasons['Spring'] . $seasons['Summer'] . $seasons['Fall'] . $seasons['Winter']);
                if ($seasons_check == '0000'){
                    form_set_error('plantation][season', 'Seasons: field is required.');
                }

                $assessions = $plantation['assessions'];
                if ($assessions == '0'){
                    form_set_error('plantation][assessions', 'Number of times the populations were assessed: field is required.');
                }

                if ($treatment['check'] == '1'){
                    $selected = false;
                    $description = false;
                    $treatment_empty = true;

                    foreach ($treatment as $field => $value){
                        if ($field != 'check'){
                            if (!$description){
                                $description = true;
                                $selected = $value;
                                if ($value == '1'){
                                    $treatment_empty = false;
                                }
                                continue;
                            }
                            elseif ($selected == '1' and $value == ''){
                                form_set_error("plantation][treatment][$field", "$field: field is required.");
                            }
                            $description = false;
                        }
                    }

                    if ($treatment_empty){
                        form_set_error("plantation][treatment][check", 'Treatment: field is required.');
                    }
                }

                break;
            default:
                form_set_error('');
                break;
        }
    }
}

function page_2_submit_form(&$form, &$form_state){
    
}
