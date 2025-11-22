<?php
/**
 * Implements hook_drush_command().
 */
function tpps_drush_command() {
    $commands['tpps-submit-command'] = array(
        'description' => 'This is TPPS Submit command',
        'aliases' => array('tpps-submit'),
        'arguments' => array(
           'section' => 'resubmit, genotype, phenotype, cleardb',
           'action' => 'These depends on the section you choose',
           'values' => 'Specific commands based on the action you request (comma separated)'
        ),
         'options' => array(
           // 'opt1' => 'My custom option.', // this --opt1=blah
        ),
        'examples' => array(
           'drush test' => 'Print my example command.',
           //'drush test myargument' => 'Print my example command with an argument "myargument".',
           //'drush test myargument --opt1=myoption' => 'Print my example command with an argument "myargument" and an option "myoption".',
        ),
    );
    return $commands;
}


function drush_tpps_submit_command($section = NULL, $action = NULL, $values = NULL) {
    // This helps user determine what they want to run
    // if they forget to specify action
    if (!isset($section)) {
        $section_list = '';
        $section_list .= 'genotype';
        $section_list .= ',phenotype';
        $section_list .= ',cleardb';
        echo 'Specify a section: ' . $section_list . "\n";
    }

    if (isset($section) && !isset($action)) {
        $action_list = '';
        switch ($section) {
            case 'resubmit':
                $action_list .= 'all';
                break;
            case 'cleardb':
                $action_list .= 'all, extra';
                break;
            case 'genotype':
                $action_list .= 'vcf_generate_sample_file';
                break;
            case 'phenotype':
                $action_list .= 'No commands available as yet';
                break;
        }
        echo 'Specify an action: ' . $action_list . "\n";
    }

    if (isset($section) && isset($action)) {
        $values = explode(',', $values);
        $values_key_vals = [];
        foreach ($values as $value_pair) {
            if (!empty($value_pair)) {
                $value_pair = explode('=', $value_pair);
                $values_key_vals[$value_pair[0]] = $value_pair[1];
            }
        }
        
        // This will remove the array entirely if it's empty
        // We want this so our functions don't assume an empty array has values
        if (count($values_key_vals) == 0) {
            unset($values_key_vals);
        }

        switch ($section) {
            case 'resubmit': {
                switch ($action) {
                    case 'all': {
                        tpps_run_tpps_submission_resubmit_all($action, $values_key_vals);
                        break;                        
                    }
                }
                break;
            }
            case 'cleardb':
                switch ($action) {
                    case 'all':
                        $transaction = db_transaction();
                        try {
                            tpps_run_tpps_submission_clear_db($action, $values_key_vals);
                        }
                        catch (\Exception $e) {
                          $transaction->rollback();
                          throw $e;
                        }
                        break;
                    case 'extra':
                        $transaction = db_transaction();
                        try {
                            tpps_run_tpps_submission_clear_db_extra($action, $values_key_vals);
                        }
                        catch (\Exception $e) {
                          $transaction->rollback();
                          throw $e;
                        }
                        break;                        
                }
                break;
            case 'genotype':
                switch ($action) {
                    case 'vcf_generate_sample_file':
                        tpps_run_vcf_generate_sample_file($action, $values_key_vals);
                        break;
                }
                break;
            case 'phenotype':
                break;
        }
    }
}

function tpps_run_tpps_submission_resubmit_all($action = NULL, $values = NULL) {
    // print_r($values);
    if (isset($values)) {
        // print_r($values);
        include_once('forms/submit/submit_all.php');
        tpps_submit_all($values['accession'], NULL);
    }
    else {
        echo $action . " - No values were specified\n";
        $options_list = "";
        $options_list .= "accession=<TGDRxxx>";
        echo $action . " - Options available: " . $options_list . "\n";
    }
}

function tpps_run_tpps_submission_clear_db_extra($action = NULL, $values = NULL) {
    // print_r($values);
    if (isset($values)) {
        // print_r($values);
        include_once('forms/submit/submit_all.php');
        tpps_submission_clear_db_extra([
            $values['accession']
        ]);
    }
    else {
        echo $action . " - No values were specified\n";
        $options_list = "";
        $options_list .= "accession=<TGDRxxx>";
        echo $action . " - Options available: " . $options_list . "\n";
    }
}

function tpps_run_tpps_submission_clear_db($action = NULL, $values = NULL) {
    // print_r($values);
    if (isset($values)) {
        // print_r($values);
        include_once('forms/submit/submit_all.php');
        tpps_submission_clear_db([
            $values['accession']
        ]);
    }
    else {
        echo $action . " - No values were specified\n";
        $options_list = "";
        $options_list .= "accession=<TGDRxxx>";
        echo $action . " - Options available: " . $options_list . "\n";
    }
}

function tpps_run_vcf_generate_sample_file($action = NULL, $values = NULL) {
    // print_r($values);
    if (isset($values)) {
        // print_r($values);
        include_once('forms/submit/submit_all.php');
        tpps_generate_genotype_sample_file_from_vcf([
            'study_accession' => $values['accession']
        ]);
    }
    else {
        echo $action . " - No values were specified\n";
        $options_list = "";
        $options_list .= "accession=<TGDRxxx>";
        echo $action . " - Options available: " . $options_list . "\n";
    }
}
?>