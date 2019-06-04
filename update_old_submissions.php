<?php


function update_old_submissions($form, $form_state){
  $output = "";
  $query = db_select('variable', 'v')
    ->fields('v')
    ->condition('name', db_like('tpps_complete_') . '%', 'LIKE')
    ->execute();

  while (($result = $query->fetchObject())){
    $name = $result->name;
    $mail = substr($result->name, 14, -7);
    $state = unserialize($result->value);
    $user = user_load_by_mail($mail);
    if ($user)
      $uid = $user->uid;
    $accession = $state['accession'];
    $dbxref_id = $state['dbxref_id'];
    $status = $state['status'];

    if (array_key_exists('Hellopage', $state['saved_values'])){
      $state['saved_values'][1] = $state['saved_values']['Hellopage'];
      $state['saved_values'][2] = $state['saved_values']['secondPage'];
      $state['saved_values'][3] = $state['saved_values']['thirdPage'];
      $state['saved_values'][4] = $state['saved_values']['fourthPage'];
      unset($state['saved_values']['Hellopage']);
      unset($state['saved_values']['secondPage']);
      unset($state['saved_values']['thirdPage']);
      unset($state['saved_values']['fourthPage']);
    }
    $state['saved_values'][1]['step'] = 1;
    $state['saved_values'][2]['step'] = 2;
    $state['saved_values'][3]['step'] = 3;
    $state['saved_values'][4]['step'] = 4;

    for ($i = 1; $i <= $state['saved_values'][1]['organism']['number']; $i++){
      if (!empty($state['saved_values'][1]['organism'][$i]['species'])){
        $state['saved_values'][1]['organism'][$i] = $state['saved_values'][1]['organism'][$i]['species'];
      }
    }

    $page_2 = &$state['saved_values'][2];
    $page_2['study_location'] = $page_2['studyLocation'];
    unset($page_2['studyLocation']);
    if (strlen($page_2['dataType']) == 1){
      $data_options = array(
        '- Select -',
        'Genotype x Phenotype',
        'Genotype',
        'Genotype x Phenotype x Environment',
        'Phenotype x Environment',
        'Genotype x Environment'
      );
      $page_2['data_type'] = $data_options[$page_2['dataType']];
    }
    else {
      $page_2['data_type'] = $page_2['dataType'];
    }
    unset($page_2['dataType']);
    $page_2['study_type'] = $page_2['studyType'];
    unset($page_2['studyType']);
    switch ($page_2['study_type']){
      case '1':
        $nat_pop = &$page_2['naturalPopulation'];
        foreach($nat_pop['season'] as $season){
          if ($season){
            $page_2['study_info']['season'][$season] = $season;
          }
        }
        $page_2['study_info']['assessions'] = $nat_pop['assessions'];
        break;
      case '2':
        $growth = &$page_2['growthChamber'];
        $page_2['study_info']['co2'] = $growth['co2Control'];
        $page_2['study_info']['humidity'] = $growth['humidityControl'];
        $page_2['study_info']['light'] = $growth['lightControl'];
        $page_2['study_info']['temp'] = $growth['temp'];
        $page_2['study_info']['rooting'] = $growth['rooting'];
        break;
      case '3':
        $green = &$page_2['greenhouse'];
        $page_2['study_info']['humidity'] = $green['humidityControl'];
        $page_2['study_info']['light'] = $green['lightControl'];
        $page_2['study_info']['temp'] = $green['temp'];
        $page_2['study_info']['rooting'] = $green['rooting'];
        break;
      case '4':
        $common = &$page_2['commonGarden'];
        $page_2['study_info']['irrigation'] = $common['irrigation'];
        $page_2['study_info']['salinity'] = $common['salinity'];
        $page_2['study_info']['biotic_env'] = $common['bioticEnv'];
        foreach($common['season'] as $season){
          if ($season){
            $page_2['study_info']['season'][$season] = $season;
          }
        }
        $page_2['study_info']['treatment'] = $common['treatment'];
        break;
      case '5':
        $plant = &$page_2['plantation'];
        foreach($plant['season'] as $season){
          if ($season){
            $page_2['study_info']['season'][$season] = $season;
          }
        }
        $page_2['study_info']['assessions'] = $plant['assessions'];
        $page_2['study_info']['treatment'] = $plant['treatment'];
        break;
      default:
        break;
    }
    unset($page_2['naturalPopulation']);
    unset($page_2['growthChamber']);
    unset($page_2['greenhouse']);
    unset($page_2['commonGarden']);
    unset($page_2['plantation']);

    dpm($state);
    variable_set($name, $state);
  }

  return $form;
}
