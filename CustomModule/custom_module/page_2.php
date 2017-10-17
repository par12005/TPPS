<?php
function page_2_create_form(&$form){
    
    function studyDate($type, &$form){
        
        $form[$type . 'Date'] = array(
          '#prefix' => '<div class="container-inline">', 
          '#type' => 'fieldset',
          '#title' => t($type . ' Date'),
          '#tree' => TRUE,
        );
        
        $yearArr = array();
        $yearArr[0] = '-Select-';
        for ($i = 1950; $i <= 2017; $i++) {
            $index = $i - 1949;
            $yearArr[$index] = $i;
        }
        

        $form[$type . 'Date']['year'] = array(
          '#type' => 'select',
          '#title' => t('Year:'),
          '#options' => $yearArr,
          '#default_value' => 0,
          '#required' => true,
        );
        
        $form[$type . 'Date']['month'] = array(
          '#type' => 'select',
          '#title' => t('Month:'),
          '#options' => array(
            0 => '-Select-',
            1 => 'January',
            2 => 'February',
            3 => 'March',
            4 => 'April',
            5 => 'May',
            6 => 'June',
            7 => 'July',
            8 => 'August',
            9 => 'September',
            10 => 'October',
            11 => 'November',
            12 => 'December'
          ),
          '#default_value' => 0,
          '#required' => true,
          '#suffix' => '</div>',
          '#states' => array(
            'invisible' => array(
              ':input[name="' . $type . 'Date[year]"]' => array('value' => '0')
            )
          )
        );
        
        return $form;
    }
    
    function studyLocation(&$form){
        
        $form['studyLocation'] = array(
          '#type' => 'fieldset',
          '#title' => t('Study Location:'),
          '#tree' => TRUE,
        );
        
        $form['studyLocation']['type'] = array(
          '#type' => 'select',
          '#title' => t('Location Format:'),
          '#options' => array(
            0 => 'Please select a location type',
            1 => 'Latitude/Longitude (WGS 84)',
            3 => 'Latitude/Longitude (NAD 83)',
            4 => 'Latitude/Longitude (ETRS 89)',
            2 => 'Custom Location'
          ),
          '#default_value' => 0,
          '#required' => TRUE
        );

        $form['studyLocation']['latitude'] = array(
          '#type' => 'textfield',
          '#title' => t('Latitude:'),
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
            'required' => array(
              array(
                array(':input[name="studyLocation[type]"]' => array('value' => '1')),
                'or',
                array(':input[name="studyLocation[type]"]' => array('value' => '3')),
                'or',
                array(':input[name="studyLocation[type]"]' => array('value' => '4')),
              )
            ),
          ),
        );

        $form['studyLocation']['longitude'] = array(
          '#type' => 'textfield',
          '#title' => t('Longitude:'),
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
            'required' => array(
              array(
                array(':input[name="studyLocation[type]"]' => array('value' => '1')),
                'or',
                array(':input[name="studyLocation[type]"]' => array('value' => '3')),
                'or',
                array(':input[name="studyLocation[type]"]' => array('value' => '4')),
              )
            ),
          ),
        );
        
        $form['studyLocation']['customLocation'] = array(
          '#type' => 'fieldset',
        );

        $form['studyLocation']['customLocation']['country'] = array(
          '#type' => 'textfield',
          '#title' => t('Country:'),
          '#states' => array(
            'required' => array(
              ':input[name="studyLocation[type]"]' => array('value' => '2')
            ),
            'visible' => array(
              ':input[name="studyLocation[type]"]' => array('value' => '2')
            ),
          )
        );
        
        $form['studyLocation']['customLocation']['region'] = array(
          '#type' => 'textfield',
          '#title' => t('State/Province/Region:'),
          '#states' => array(
            'invisible' => array(
              ':input[name="studyLocation[customLocation][country]"]' => array('empty' => true)
            )
          )
        );
        
        return $form;
    }
    
    function naturalPopulation(&$form){
        
        $form['naturalPopulation'] = array(
          '#type' => 'fieldset',
          '#title' => t('Natural Population/Landscape Information:'),
          '#tree' => TRUE,
          '#states' => array(
            'visible' => array(
              ':input[name="studyType"]' => array('value' => '1')
            ),
            'required' => array(
              ':input[name="studyType"]' => array('value' => '1')
            ),
            'enabled' => array(
              ':input[name="studyType"]' => array('value' => '1')
            )
          )
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
        
        $form['naturalPopulation']['assessions'] = array(
          '#type' => 'textfield',
          '#title' => t('Number of times the populations were assessed (on average):')
        );
        
        return $form;
    }
    
    function growthChamber(&$form){
        
        function co2(&$form){
            
            $form['growthChamber']['co2Control'] = array(
              '#type' => 'fieldset',
              '#tree' => true
            );

            $form['growthChamber']['co2Control']['option'] = array(
              '#type' => 'select',
              '#title' => t('CO2 controlled or uncontrolled'),
              '#options' => array(
                0 => 'Please select CO2 control',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => 0,
              '#states' => array(
                'required' => array(
                  ':input[name="studyType"]' => array('value' => '2')
                )
              )
            );

            $form['growthChamber']['co2Control']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled CO2 Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[co2Control][option]"]' => array('value' => '1')
                ),
                'required' => array(
                  ':input[name="growthChamber[co2Control][option]"]' => array('value' => '1')
                )
              )
            );

            $form['growthChamber']['co2Control']['uncontrolled'] = array(
              '#type' => 'textfield',
              '#title' => t('Average CO2 Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[co2Control][option]"]' => array('value' => '2')
                ),
                'required' => array(
                  ':input[name="growthChamber[co2Control][option]"]' => array('value' => '2')
                )
              )
            );
            
            return $form;
        }
        
        function humidity(&$form){
            
            $form['growthChamber']['humidityControl'] = array(
              '#type' => 'fieldset',
              '#tree' => true,
            );

            $form['growthChamber']['humidityControl']['option'] = array(
              '#type' => 'select',
              '#title' => t('Air Humidity controlled or uncontrolled'),
              '#options' => array(
                0 => 'Please select Air Humidity control',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => 0,
              '#states' => array(
                'required' => array(
                  ':input[name="studyType"]' => array('value' => '2')
                )
              )
            );

            $form['growthChamber']['humidityControl']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled Air Humidity Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[humidityControl][option]"]' => array('value' => '1')
                ),
                'required' => array(
                  ':input[name="growthChamber[humidityControl][option]"]' => array('value' => '1')
                )
              )
            );

            $form['growthChamber']['humidityControl']['uncontrolled'] = array(
              '#type' => 'textfield',
              '#title' => t('Average Air Humidity Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[humidityControl][option]"]' => array('value' => '2')
                ),
                'required' => array(
                  ':input[name="growthChamber[humidityControl][option]"]' => array('value' => '2')
                )
              )
            );
            
            return $form;
        }
        
        function light(&$form){
            
            $form['growthChamber']['lightControl'] = array(
              '#type' => 'fieldset',
              '#tree' => true,
            );

            $form['growthChamber']['lightControl']['option'] = array(
              '#type' => 'select',
              '#title' => t('Light Intensity controlled or uncontrolled'),
              '#options' => array(
                0 => 'Please select Light Intensity control',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => 0,
              '#states' => array(
                'required' => array(
                  ':input[name="studyType"]' => array('value' => '2')
                )
              )
            );

            $form['growthChamber']['lightControl']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled Light Intensity Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[lightControl][option]"]' => array('value' => '1')
                ),
                'required' => array(
                  ':input[name="growthChamber[lightControl][option]"]' => array('value' => '1')
                )
              )
            );

            $form['growthChamber']['lightControl']['uncontrolled'] = array(
              '#type' => 'textfield',
              '#title' => t('Average Light Intensity Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[lightControl][option]"]' => array('value' => '2')
                ),
                'required' => array(
                  ':input[name="growthChamber[lightControl][option]"]' => array('value' => '2')
                )
              )
            );
            
            return $form;
        }
        
        function ph(&$form){
            
            $form['growthChamber']['rooting']['ph'] = array(
              '#type' => 'fieldset',
              '#tree' => true
            );

            $form['growthChamber']['rooting']['ph']['option'] = array(
              '#type' => 'select',
              '#title' => t('pH controlled or uncontrolled'),
              '#options' => array(
                0 => 'Please select pH control',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => 0,
              '#states' => array(
                'required' => array(
                  ':input[name="studyType"]' => array('value' => '2')
                )
              )
            );

            $form['growthChamber']['rooting']['ph']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled pH Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[rooting][ph][option]"]' => array('value' => '1')
                ),
                'required' => array(
                  ':input[name="growthChamber[rooting][ph][option]"]' => array('value' => '1')
                )
              )
            );

            $form['growthChamber']['rooting']['ph']['uncontrolled'] = array(
              '#type' => 'textfield',
              '#title' => t('Average pH Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[rooting][ph][option]"]' => array('value' => '2')
                ),
                'required' => array(
                  ':input[name="growthChamber[rooting][ph][option]"]' => array('value' => '2')
                )
              )
            );
            
            return $form;
        }
        
        $form['growthChamber'] = array(
          '#type' => 'fieldset',
          '#title' => t('Growth Chamber Information:'),
          '#tree' => TRUE,
          '#states' => array(
            'visible' => array(
              ':input[name="studyType"]' => array('value' => '2')
            ),
            'enabled' => array(
              ':input[name="studyType"]' => array('value' => '2')
            )
          )
        );
        
        co2($form);
        
        humidity($form);
        
        light($form);

        $form['growthChamber']['temp'] = array(
          '#type' => 'fieldset',
          '#title' => t('Temperature Information:'),
          '#description' => t('Please provide temperatures in Degrees Celsius'),
          '#tree' => true,
          '#states' => array(
            'required' => array(
              ':input[name="studyType"]' => array('value' => '2')
            )
          )
        );

        $form['growthChamber']['temp']['high'] = array(
          '#type' => 'textfield',
          '#title' => t('Average High Temperature'),
        );

        $form['growthChamber']['temp']['low'] = array(
          '#type' => 'textfield',
          '#title' => t('Average Low Temperature'),
        );
        
        $form['growthChamber']['rooting'] = array(
          '#type' => 'fieldset',
          '#title' => t('Rooting Information:'),
          '#tree' => true,
        );
        
        $form['growthChamber']['rooting']['option'] = array(
          '#type' => 'select',
          '#title' => t('Rooting Type:'),
          '#options' => array(
            0 => 'Please select a rooting type',
            1 => 'Aeroponics',
            2 => 'Hydroponics',
            3 => 'Soil',
          ),
          '#default_value' => 0,
          '#states' => array(
            'required' => array(
              ':input[name="studyType"]' => array('value' => '2')
            )
          )
        );
        
        $form['growthChamber']['rooting']['soil'] = array(
          '#type' => 'fieldset',
          '#title' => t('Soil Information:'),
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
            0 => 'Please select a soil type',
            1 => 'Sand',
            2 => 'Peat',
            3 => 'Clay',
            4 => 'Mixed',
            5 => 'Other'
          ),
          '#states' => array(
            'required' => array(
              ':input[name="growthChamber[rooting][option]"]' => array('value' => '3')
            )
          )
        );
        
        $form['growthChamber']['rooting']['soil']['other'] = array(
          '#type' => 'textfield',
          '#states' => array(
            'visible' => array(
              ':input[name="growthChamber[rooting][soil][type]"]' => array('value' => '5')
            ),
            'required' => array(
              ':input[name="growthChamber[rooting][soil][type]"]' => array('value' => '5')
            )
          )
        );
        
        $form['growthChamber']['rooting']['soil']['container'] = array(
          '#type' => 'textfield',
          '#title' => t('Soil Container Type:'),
          '#states' => array (
            'required' => array(
              ':input[name="growthChamber[rooting][option]"]' => array('value' => '3')
            )
          )
        );
        
        ph($form);
        
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
          '#title' => t('Treatments:'),
          '#states' => array(
            'required' => array(
              ':input[name="studyType"]' => array('value' => '2')
            )
          )
        );
        
        foreach($treatment_options as $key => $option){
            $form['growthChamber']['rooting']['treatment']["$option"] = array(
              '#type' => 'checkbox',
              '#title' => t("$option")
            );
            $form['growthChamber']['rooting']['treatment']["$option-description"] = array(
              '#type' => 'textfield',
              '#description' => t("$option Description"),
              '#states' => array(
                'visible' => array(
                  ':input[name="growthChamber[rooting][treatment][' . $option . ']"]' => array('checked' => TRUE)
                ),
                'required' => array(
                  ':input[name="growthChamber[rooting][treatment][' . $option . ']"]' => array('checked' => TRUE)
                )
              )
            );
        }
        
        return $form;
    }
    
    function greenhouse(&$form){
        
        function greenhumidity(&$form){
            
            $form['greenhouse']['humidityControl'] = array(
              '#type' => 'fieldset',
              '#tree' => true,
            );

            $form['greenhouse']['humidityControl']['option'] = array(
              '#type' => 'select',
              '#title' => t('Air Humidity controlled or uncontrolled'),
              '#options' => array(
                0 => 'Please select air humidity control',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => 0,
              '#states' => array(
                'required' => array(
                  ':input[name="studyType"]' => array('value' => '3')
                )
              )
            );

            $form['greenhouse']['humidityControl']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled Air Humidity Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="greenhouse[humidityControl][option]"]' => array('value' => '1')
                ),
                'required' => array(
                  ':input[name="greenhouse[humidityControl][option]"]' => array('value' => '1')
                )
              )
            );
            
            return $form;
        }
        
        function greenlight(&$form){
            
            $form['greenhouse']['lightControl'] = array(
              '#type' => 'fieldset',
              '#tree' => true,
            );

            $form['greenhouse']['lightControl']['option'] = array(
              '#type' => 'select',
              '#title' => t('Light Intensity controlled or uncontrolled'),
              '#options' => array(
                0 => 'Please select Light Intensity control',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => 0,
              '#states' => array(
                'required' => array(
                  ':input[name="studyType"]' => array('value' => '3')
                )
              )
            );

            $form['greenhouse']['lightControl']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled Light Intensity Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="greenhouse[lightControl][option]"]' => array('value' => '1')
                ),
                'required' => array(
                  ':input[name="greenhouse[lightControl][option]"]' => array('value' => '1')
                )
              )
            );
            
            return $form;
        }
        
        function greenph(&$form){
            
            $form['greenhouse']['rooting']['ph'] = array(
              '#type' => 'fieldset',
              '#tree' => true
            );

            $form['greenhouse']['rooting']['ph']['option'] = array(
              '#type' => 'select',
              '#title' => t('pH controlled or uncontrolled'),
              '#options' => array(
                0 => 'Please select pH control',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => 0,
              '#states' => array(
                'required' => array(
                  ':input[name="studyType"]' => array('value' => '3')
                )
              )
            );

            $form['greenhouse']['rooting']['ph']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled pH Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="greenhouse[rooting][ph][option]"]' => array('value' => '1')
                ),
                'required' => array(
                  ':input[name="greenhouse[rooting][ph][option]"]' => array('value' => '1')
                )
              )
            );
            
            return $form;
        }
        
        $form['greenhouse'] = array(
          '#type' => 'fieldset',
          '#title' => t('Greenhouse Information:'),
          '#tree' => TRUE,
          '#states' => array(
            'visible' => array(
              ':input[name="studyType"]' => array('value' => '3')
            ),
            'enabled' => array(
              ':input[name="studyType"]' => array('value' => '3')
            )
          )
        );
        
        greenhumidity($form);
        
        greenlight($form);

        $form['greenhouse']['temp'] = array(
          '#type' => 'fieldset',
          '#title' => t('Temperature Information:'),
          '#description' => t('Please provide temperatures in Degrees Celsius'),
          '#tree' => true,
          '#states' => array(
            'required' => array(
              ':input[name="studyType"]' => array('value' => '3')
            )
          )
        );

        $form['greenhouse']['temp']['high'] = array(
          '#type' => 'textfield',
          '#title' => t('Average High Temperature:'),
        );

        $form['greenhouse']['temp']['low'] = array(
          '#type' => 'textfield',
          '#title' => t('Average Low Temperature:'),
        );
        
        $form['greenhouse']['rooting'] = array(
          '#type' => 'fieldset',
          '#title' => t('Rooting Information:'),
          '#tree' => true,
        );
        
        $form['greenhouse']['rooting']['option'] = array(
          '#type' => 'select',
          '#title' => t('Rooting Type:'),
          '#options' => array(
            0 => 'Please select a rooting type',
            1 => 'Aeroponics',
            2 => 'Hydroponics',
            3 => 'Soil',
          ),
          '#default_value' => 0,
          '#states' => array(
            'required' => array(
              ':input[name="studyType"]' => array('value' => '3')
            )
          )
        );
        
        $form['greenhouse']['rooting']['soil'] = array(
          '#type' => 'fieldset',
          '#title' => t('Soil Information:'),
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
            0 => 'Please select a soil type',
            1 => 'Sand',
            2 => 'Peat',
            3 => 'Clay',
            4 => 'Mixed',
            5 => 'Other'
          ),
          '#states' => array(
            'required' => array(
              ':input[name="greenhouse[rooting][option]"]' => array('value' => '3')
            )
          )
        );
        
        $form['greenhouse']['rooting']['soil']['other'] = array(
          '#type' => 'textfield',
          '#states' => array(
            'visible' => array(
              ':input[name="greenhouse[rooting][soil][type]"]' => array('value' => '5')
            ),
            'required' => array(
              ':input[name="greenhouse[rooting][soil][type]"]' => array('value' => '5')
            )
          )
        );
        
        $form['greenhouse']['rooting']['soil']['container'] = array(
          '#type' => 'textfield',
          '#title' => t('Soil Container Type:'),
          '#states' => array (
            'required' => array(
              ':input[name="greenhouse[rooting][option]"]' => array('value' => '3')
            )
          )
        );
        
        greenph($form);
        
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
          '#title' => t('Treatments:'),
          '#states' => array(
            'required' => array(
              ':input[name="studyType"]' => array('value' => '3')
            )
          )
        );
        
        foreach($treatment_options as $key => $option){
            $form['greenhouse']['rooting']['treatment']["$option"] = array(
              '#type' => 'checkbox',
              '#title' => t("$option")
            );
            $form['greenhouse']['rooting']['treatment']["$option-description"] = array(
              '#type' => 'textfield',
              '#description' => t("$option Description"),
              '#states' => array(
                'visible' => array(
                  ':input[name="greenhouse[rooting][treatment][' . $option . ']"]' => array('checked' => TRUE)
                ),
                'required' => array(
                  ':input[name="greenhouse[rooting][treatment][' . $option . ']"]' => array('checked' => TRUE)
                )
              )
            );
        }
        
        return $form;
    }
    
    function commonGarden(&$form){
        
        function salinity(&$form){
            
            $form['commonGarden']['salinity'] = array(
              '#type' => 'fieldset',
              '#tree' => true
            );

            $form['commonGarden']['salinity']['option'] = array(
              '#type' => 'select',
              '#title' => t('Salinity controlled or uncontrolled'),
              '#options' => array(
                0 => 'Please select Salinity control',
                1 => 'Controlled',
                2 => 'Uncontrolled'
              ),
              '#default_value' => 0,
              '#states' => array(
                'required' => array(
                  ':input[name="studyType"]' => array('value' => '4')
                )
              )
            );

            $form['commonGarden']['salinity']['controlled'] = array(
              '#type' => 'textfield',
              '#title' => t('Controlled Salinity Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="commonGarden[salinity][option]"]' => array('value' => '1')
                ),
                'required' => array(
                  ':input[name="commonGarden[salinity][option]"]' => array('value' => '1')
                )
              )
            );

            $form['commonGarden']['salinity']['uncontrolled'] = array(
              '#type' => 'textfield',
              '#title' => t('Average Salinity Value:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="commonGarden[salinity][option]"]' => array('value' => '2')
                ),
                'required' => array(
                  ':input[name="commonGarden[salinity][option]"]' => array('value' => '2')
                )
              )
            );
            
            return $form;
        }
        
        $form['commonGarden'] = array(
          '#type' => 'fieldset',
          '#title' => t('Common Garden Information:'),
          '#tree' => TRUE,
          '#states' => array(
            'visible' => array(
              ':input[name="studyType"]' => array('value' => '4')
            ),
            'enabled' => array(
              ':input[name="studyType"]' => array('value' => '4')
            )
          )
        );
        
        $form['commonGarden']['irrigation'] = array(
          '#type' => 'fieldset',
          '#tree' => true,
        );
        
        $form['commonGarden']['irrigation']['option'] = array(
          '#type' => 'select',
          '#title' => t('Irrigation Type:'),
          '#options' => array(
            0 => 'Please select an irrigation type',
            1 => 'Irrigation from top',
            2 => 'Irrigation from bottom',
            3 => 'Drip Irrigation',
            4 => 'Other',
            5 => 'No Irrigation',
          ),
          '#default_value' => 0,
          '#states' => array(
            'required' => array(
              ':input[name="studyType"]' => array('value' => '4')
            )
          )
        );
        
        $form['commonGarden']['irrigation']['other'] = array(
          '#type' => 'textfield',
          '#states' => array(
            'visible' => array(
              ':input[name="commonGarden[irrigation][option]"]' => array('value' => '4')
            ),
            'required' => array(
              ':input[name="commonGarden[irrigation][option]"]' => array('value' => '4')
            )
          )
        );
        
        salinity($form);
        
        $form['commonGarden']['bioticEnv'] = array(
          '#type' => 'fieldset',
          '#tree' => true,
          '#states' => array(
            'required' => array(
              ':input[name="studyType"]' => array('value' => '4')
            )
          )
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
        
        $form['commonGarden']['bioticEnv']['other'] = array(
          '#type' => 'textfield',
          '#title' => t('Please specify Biotic Environment Type:'),
          '#states' => array(
            'visible' => array(
              ':input[name="commonGarden[bioticEnv][option][Other]"]' => array('checked' => TRUE)
            ),
            'required' => array(
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
          '#states' => array(
            'required' => array(
              ':input[name="studyType"]' => array('value' => '4')
            )
          )
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
          '#title' => t('Treatments:'),
          '#states' => array(
            'required' => array(
              ':input[name="studyType"]' => array('value' => '4')
            )
          )
        );
        
        foreach($treatment_options as $key => $option){
            $form['commonGarden']['treatment']["$option"] = array(
              '#type' => 'checkbox',
              '#title' => t("$option")
            );
            $form['commonGarden']['treatment']["$option-description"] = array(
              '#type' => 'textfield',
              '#description' => t("$option Description"),
              '#states' => array(
                'visible' => array(
                  ':input[name="commonGarden[treatment][' . $option . ']"]' => array('checked' => TRUE)
                ),
                'required' => array(
                  ':input[name="commonGarden[treatment][' . $option . ']"]' => array('checked' => TRUE)
                )
              )
            );
        }
        
        return $form;
    }
    
    studyDate('Starting', $form);
    
    studyDate('Ending', $form);
    
    studyLocation($form);

    $form['studyType'] = array(
      '#type' => 'select',
      '#title' => t('Study Type:'),
      '#options' => array(
        0 => 'Please select the type of study',
        1 => 'Natural Population (Landscape)',
        2 => 'Growth Chamber',
        3 => 'Greenhouse',
        4 => 'Experimental/Common Garden',
        5 => 'Plantation',
      ),
      '#default_value' => 0,
      '#required' => true,
    );
    
    naturalPopulation($form);
    
    growthChamber($form);
    
    greenhouse($form);
    
    commonGarden($form);
    
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );

    drupal_add_js(drupal_get_path('module', 'custom_module') . "/custom_module.js");

    return $form;
}

function page_2_validate_form(&$form, &$form_state){
    
    $form_values = $form_state['values'];
    $start_date = $form_values['StartingDate'];
    $end_date = $form_values['EndingDate'];
    $location_type = $form_values['studyLocation']['type'];
    $latitude = $form_values['studyLocation']['latitude'];
    $longitude = $form_values['studyLocation']['longitude'];
    $custom_country = $form_values['studyLocation']['customLocation']['country'];
    $study_type = $form_values['studyType'];
    $natural_population = $form_values['naturalPopulation'];
    $growth_chamber = $form_values['growthChamber'];
    $greenhouse = $form_values['greenhouse'];
    $common_garden = $form_values['commonGarden'];
    
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
        if ($latitude == ''){
            form_set_error('studyLocation][latitude', 'Latitude: field is required.');
        }
        if ($longitude == ''){
            form_set_error('studyLocation][longitude', 'Longitude: field is required.');
        }
    }
    else{
        if($custom_country == ''){
            form_set_error('studyLocation][customLocation][country', 'Country: field is required.');
        }
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
            if ($assessions == ''){
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
            
            $selected = false;
            $description = false;

            foreach ($treatment as $field => $value){
                if (!$description){
                    $description = true;
                    $selected = $value;
                    continue;
                }
                elseif ($selected == '1' and $value == ''){
                    form_set_error("commonGarden][treatment][$field", "$field: field is required.");
                }
                $description = false;
            }
            
            break;
        case '5':
            //Plantation
            break;
        default:
            form_set_error('');
            break;
    }
}

function page_2_submit_form(&$form, &$form_state){
    /*$startMonth = $form_state['values']['startingDate'][1]['Month'];
   $startYear = $form_state['values']['startingDate'][1]['Year'];
   $startDay = $form_state['values']['startingDate'][1]['Day'];
   
   $endMonth = $form_state['values']['endingDate'][1]['Month'];
   $endYear = $form_state['values']['endingDate'][1]['Year'];
   $endDay = $form_state['values']['endingDate'][1]['Day'];
   
   $location = $form_state['values']['location'];
   $studyType = $form_state['values']['studyType'];*/
    
   $form_state['redirect'] = 'thirdPage';
   
   
   
   
   
   

//    To be implemented
}
