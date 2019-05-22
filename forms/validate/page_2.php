<?php

/**
 * @file
 */

/**
 *
 */
function page_2_validate_form(&$form, &$form_state) {
  if ($form_state['submitted'] == '1') {
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

    if ($start_date['year'] == '0') {
      form_set_error('StartingDate][year', 'Year: field is required.');
    }
    elseif ($start_date['month'] == '0') {
      form_set_error('StartingDate][month', 'Month: field is required.');
    }

    if ($end_date['year'] == '0') {
      form_set_error('EndingDate][year', 'Year: field is required.');
    }
    elseif ($end_date['month'] == '0') {
      form_set_error('EndingDate][month', 'Month: field is required.');
    }

    if ($location_type == '0') {
      form_set_error('studyLocation][type', 'Location Format: field is required.');
    }
    elseif ($location_type == '1' or $location_type == '3' or $location_type == '4') {
      if ($coordinates == '') {
        form_set_error('studyLocation][coordinates', 'Coordinates: field is required.');
      }
    }
    else {
      if ($custom_location == '') {
        form_set_error('studyLocation][custom', 'Custom Location: field is required.');
      }
    }

    if (!$data_type) {
      form_set_error('dataType', 'Data Type: field is required.');
    }

    switch ($study_type) {
      case '0':
        form_set_error('studyType', 'Study Type: field is required.');
        break;

      case '1':
        // Natural population.
        $seasons = $natural_population['season'];
        $seasons_check = ($seasons['Spring'] . $seasons['Summer'] . $seasons['Fall'] . $seasons['Winter']);
        if ($seasons_check == '0000') {
          form_set_error('naturalPopulation][season', 'Seasons: field is required.');
        }

        $assessions = $natural_population['assessions'];
        if ($assessions == '0') {
          form_set_error('naturalPopulation][assessions', 'Number of times the populations were assessed: field is required.');
        }
        break;

      case '2':
        // Growth chamber.
        $temp_high = $growth_chamber['temp']['high'];
        $temp_low = $growth_chamber['temp']['low'];

        /**
         *
         */
        function valid_co2($growth_chamber) {
          $co2_control = $growth_chamber['co2Control']['option'];
          $co2_controlled = $growth_chamber['co2Control']['controlled'];
          $co2_uncontrolled = $growth_chamber['co2Control']['uncontrolled'];
          if ($co2_control == '0') {
            form_set_error('growthChamber][co2Control][option', 'CO2 controlled or uncontrolled: field is required.');
          }
          elseif ($co2_control == '1' and $co2_controlled == '') {
            form_set_error('growthChamber][co2Control][controlled', 'Controlled CO2 Value: field is required.');
          }
          elseif ($co2_control == '2' and $co2_uncontrolled == '') {
            form_set_error('growthChamber][co2Control][uncontrolled', 'Average CO2 Value: field is required.');
          }
        }

        /**
         *
         */
        function valid_humidity($growth_chamber) {
          $humidity_control = $growth_chamber['humidityControl']['option'];
          $humidity_controlled = $growth_chamber['humidityControl']['controlled'];
          $humidity_uncontrolled = $growth_chamber['humidityControl']['uncontrolled'];
          if ($humidity_control == '0') {
            form_set_error('growthChamber][humidityControl][option', 'Air Humidity controlled or uncontrolled: field is required.');
          }
          elseif ($humidity_control == '1' and $humidity_controlled == '') {
            form_set_error('growthChamber][humidityControl][controlled', 'Controlled Air Humidity Value: field is required.');
          }
          elseif ($humidity_control == '2' and $humidity_uncontrolled == '') {
            form_set_error('growthChamber][humidityControl][uncontrolled', 'Average Air Humidity Value: field is required.');
          }
        }

        /**
         *
         */
        function valid_light($growth_chamber) {
          $light_control = $growth_chamber['lightControl']['option'];
          $light_controlled = $growth_chamber['lightControl']['controlled'];
          $light_uncontrolled = $growth_chamber['lightControl']['uncontrolled'];
          if ($light_control == '0') {
            form_set_error('growthChamber][lightControl][option', 'Light Intensity controlled or uncontrolled: field is required.');
          }
          elseif ($light_control == '1' and $light_controlled == '') {
            form_set_error('growthChamber][lightControl][controlled', 'Controlled Light Intensity Value: field is required.');
          }
          elseif ($light_control == '2' and $light_uncontrolled == '') {
            form_set_error('growthChamber][lightControl][uncontrolled', 'Average Light Intensity Value: field is required.');
          }
        }

        /**
         *
         */
        function valid_ph($growth_chamber) {
          $ph_control = $growth_chamber['rooting']['ph']['option'];
          $ph_controlled = $growth_chamber['rooting']['ph']['controlled'];
          $ph_uncontrolled = $growth_chamber['rooting']['ph']['uncontrolled'];
          if ($ph_control == '0') {
            form_set_error('growthChamber][rooting][ph][option', 'pH controlled or uncontrolled: field is required.');
          }
          elseif ($ph_control == '1' and $ph_controlled == '') {
            form_set_error('growthChamber][rooting][ph][controlled', 'Controlled pH Value: field is required.');
          }
          elseif ($ph_control == '2' and $ph_uncontrolled == '') {
            form_set_error('growthChamber][rooting][ph][uncontrolled', 'Average pH Value: field is required.');
          }
        }

        /**
         *
         */
        function valid_rooting($growth_chamber) {
          $rooting_option = $growth_chamber['rooting']['option'];
          $soil_type = $growth_chamber['rooting']['soil']['type'];
          $soil_container = $growth_chamber['rooting']['soil']['container'];
          $soil_other = $growth_chamber['rooting']['soil']['other'];
          $treatment = $growth_chamber['rooting']['treatment'];

          if ($rooting_option == '0') {
            form_set_error('growthChamber][rooting][option', 'Rooting Type: field is required.');
          }
          elseif ($rooting_option == '3') {
            if ($soil_type == '0') {
              form_set_error('growthChamber][rooting][soil][type', 'Soil Type: field is required.');
            }
            elseif ($soil_type == '5' and $soil_other == '') {
              form_set_error('growthChamber][rooting][soil][other', 'Custom Soil Type: field is required.');
            }

            if ($soil_container == '') {
              form_set_error('growthChamber][rooting][soil][container', 'Soil Container Type: field is required.');
            }
          }

          valid_ph($growth_chamber);

          // Consider using some counter variable to take every other item in $treatment.
          $selected = FALSE;
          $description = FALSE;

          foreach ($treatment as $field => $value) {
            if (!$description) {
              $description = TRUE;
              $selected = $value;
              continue;
            }
            elseif ($selected == '1' and $value == '') {
              form_set_error("growthChamber][rooting][treatment][$field", "$field: field is required.");
            }
            $description = FALSE;
          }
        }

        valid_co2($growth_chamber);

        valid_humidity($growth_chamber);

        valid_light($growth_chamber);

        if ($temp_high == '') {
          form_set_error('growthChamber][temp][high', 'Average High Temperature: field is required.');
        }

        if ($temp_low == '') {
          form_set_error('growthChamber][temp][low', 'Average Low Temperature: field is required.');
        }

        valid_rooting($growth_chamber);

        break;

      case '3':
        // Greenhouse.
        $green_temp_high = $greenhouse['temp']['high'];
        $green_temp_low = $greenhouse['temp']['low'];

        /**
         *
         */
        function green_valid_humidity($greenhouse) {
          $humidity_control = $greenhouse['humidityControl']['option'];
          $humidity_controlled = $greenhouse['humidityControl']['controlled'];
          if ($humidity_control == '0') {
            form_set_error('greenhouse][humidityControl][option', 'Air Humidity controlled or uncontrolled: field is required.');
          }
          elseif ($humidity_control == '1' and $humidity_controlled == '') {
            form_set_error('greenhouse][humidityControl][controlled', 'Controlled Air Humidity Value: field is required.');
          }
        }

        /**
         *
         */
        function green_valid_light($greenhouse) {
          $light_control = $greenhouse['lightControl']['option'];
          $light_controlled = $greenhouse['lightControl']['controlled'];
          if ($light_control == '0') {
            form_set_error('greenhouse][lightControl][option', 'Light Intensity controlled or uncontrolled: field is required.');
          }
          elseif ($light_control == '1' and $light_controlled == '') {
            form_set_error('greenhouse][lightControl][controlled', 'Controlled Light Intensity Value: field is required.');
          }
        }

        /**
         *
         */
        function green_valid_ph($greenhouse) {
          $ph_control = $greenhouse['rooting']['ph']['option'];
          $ph_controlled = $greenhouse['rooting']['ph']['controlled'];
          if ($ph_control == '0') {
            form_set_error('greenhouse][rooting][ph][option', 'pH controlled or uncontrolled: field is required.');
          }
          elseif ($ph_control == '1' and $ph_controlled == '') {
            form_set_error('greenhouse][rooting][ph][controlled', 'Controlled pH Value: field is required.');
          }
        }

        /**
         *
         */
        function green_valid_rooting($greenhouse) {
          $rooting_option = $greenhouse['rooting']['option'];
          $soil_type = $greenhouse['rooting']['soil']['type'];
          $soil_container = $greenhouse['rooting']['soil']['container'];
          $soil_other = $greenhouse['rooting']['soil']['other'];
          $treatment = $greenhouse['rooting']['treatment'];

          if ($rooting_option == '0') {
            form_set_error('greenhouse][rooting][option', 'Rooting Type: field is required.');
          }
          elseif ($rooting_option == '3') {
            if ($soil_type == '0') {
              form_set_error('greenhouse][rooting][soil][type', 'Soil Type: field is required.');
            }
            elseif ($soil_type == '5' and $soil_other == '') {
              form_set_error('greenhouse][rooting][soil][other', 'Custom Soil Type: field is required.');
            }

            if ($soil_container == '') {
              form_set_error('greenhouse][rooting][soil][container', 'Soil Container Type: field is required.');
            }
          }

          green_valid_ph($greenhouse);

          // Consider using some counter variable to take every other item in $treatment.
          $selected = FALSE;
          $description = FALSE;

          foreach ($treatment as $field => $value) {
            if (!$description) {
              $description = TRUE;
              $selected = $value;
              continue;
            }
            elseif ($selected == '1' and $value == '') {
              form_set_error("greenhouse][rooting][treatment][$field", "$field: field is required.");
            }
            $description = FALSE;
          }
        }

        green_valid_humidity($greenhouse);

        green_valid_light($greenhouse);

        if ($green_temp_high == '') {
          form_set_error('greenhouse][temp][high', 'Average High Temperature: field is required.');
        }

        if ($green_temp_low == '') {
          form_set_error('greenhouse][temp][low', 'Average Low Temperature: field is required.');
        }

        green_valid_rooting($greenhouse);

        break;

      case '4':
        // Common garden.
        $irrigation_option = $common_garden['irrigation']['option'];
        $custom_irrigation = $common_garden['irrigation']['other'];
        $biotic_env = $common_garden['bioticEnv']['option'];
        $biotic_env_check = ($biotic_env['Herbivores'] . $biotic_env['Mutulists'] . $biotic_env['Pathogens'] . $biotic_env['Endophytes'] . $biotic_env['Other'] . $biotic_env['None']);
        $custom_biotic_env = $common_garden['bioticEnv']['other'];
        $seasons = $common_garden['season'];
        $seasons_check = ($seasons['Spring'] . $seasons['Summer'] . $seasons['Fall'] . $seasons['Winter']);
        $treatment = $common_garden['treatment'];

        /**
         *
         */
        function valid_salinity($common_garden) {
          $salinity_control = $common_garden['salinity']['option'];
          $salinity_controlled = $common_garden['salinity']['controlled'];
          $salinity_uncontrolled = $common_garden['salinity']['uncontrolled'];
          if ($salinity_control == '0') {
            form_set_error('commonGarden][salinity][option', 'Salinity controlled or uncontrolled: field is required.');
          }
          elseif ($salinity_control == '1' and $salinity_controlled == '') {
            form_set_error('commonGarden][salinity][controlled', 'Controlled Salinity Value: field is required.');
          }
          elseif ($salinity_control == '2' and $salinity_uncontrolled == '') {
            form_set_error('commonGarden][salinity][uncontrolled', 'Average Salinity Value: field is required.');
          }
        }

        if ($irrigation_option == '0') {
          form_set_error('commonGarden][irrigation][option', 'Irrigation Type: field is required.');
        }
        elseif ($irrigation_option == '4' and $custom_irrigation == '') {
          form_set_error('commonGarden][irrigation][other', 'Custom Irrigation Type: field is required.');
        }

        valid_salinity($common_garden);

        if ($biotic_env_check == '000000') {
          form_set_error('commonGarden][bioticEnv][option', 'Biotic Environment: field is required.');
        }
        elseif ($biotic_env['Other'] != '0' and $custom_biotic_env == '') {
          form_set_error('commonGarden][bioticEnv][other', 'Custom Biotic Environment: field is required.');
        }

        if ($seasons_check == '0000') {
          form_set_error('commonGarden][season', 'Seasons: field is required.');
        }

        if ($treatment['check'] == '1') {
          $selected = FALSE;
          $description = FALSE;
          $treatment_empty = TRUE;

          foreach ($treatment as $field => $value) {
            if ($field != 'check') {
              if (!$description) {
                $description = TRUE;
                $selected = $value;
                if ($value == '1') {
                  $treatment_empty = FALSE;
                }
                continue;
              }
              elseif ($selected == '1' and $value == '') {
                form_set_error("commonGarden][treatment][$field", "$field: field is required.");
              }
              $description = FALSE;
            }
          }

          if ($treatment_empty) {
            form_set_error("commonGarden][treatment", 'Treatment: field is required.');
          }
        }

        break;

      case '5':
        // Plantation.
        $seasons = $plantation['season'];
        $treatment = $plantation['treatment'];
        $seasons_check = ($seasons['Spring'] . $seasons['Summer'] . $seasons['Fall'] . $seasons['Winter']);
        if ($seasons_check == '0000') {
          form_set_error('plantation][season', 'Seasons: field is required.');
        }

        $assessions = $plantation['assessions'];
        if ($assessions == '0') {
          form_set_error('plantation][assessions', 'Number of times the populations were assessed: field is required.');
        }

        if ($treatment['check'] == '1') {
          $selected = FALSE;
          $description = FALSE;
          $treatment_empty = TRUE;

          foreach ($treatment as $field => $value) {
            if ($field != 'check') {
              if (!$description) {
                $description = TRUE;
                $selected = $value;
                if ($value == '1') {
                  $treatment_empty = FALSE;
                }
                continue;
              }
              elseif ($selected == '1' and $value == '') {
                form_set_error("plantation][treatment][$field", "$field: field is required.");
              }
              $description = FALSE;
            }
          }

          if ($treatment_empty) {
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
