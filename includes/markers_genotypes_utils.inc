<?php

/**
 * Removes / deletes a specific study's markers and genotypes
 *
 * @param string $accession
 *   The accession number of the form being submitted.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_remove_all_markers_genotypes($accession, TripalJob $job = NULL) {
    // Global variables.
    $tpps_job_logger = NULL;
    $tpps_job = NULL;
    $accession = trim($accession);

    // This function is found in submit_all.php and initializes the tpps_job_logger
    tpps_initialize_job_logger($accession, $job);

    if($accession == "") {
        throw new Exception('Accession must be valid, empty accession was provided');
    }
    tpps_log('[INFO] Performing removal of markers and genotypes on study: ' . $accession);
    $job->setInterval(1);

    $transaction = db_transaction();
    try {

        // Get Project ID
        tpps_log('[INFO] Finding project_id...');
        $sql = "";
        $sql .= "       select project_id from chado.project_dbxref where dbxref_id = (";
        $sql .= "           select dbxref_id from chado.dbxref where accession = '" . $accession . "'";
        $sql .= "       )";
        $results_project = chado_query($sql, []);
        $project_id = NULL;
        foreach ($results_project as $results_project_row) {
            $project_id = $results_project_row->project_id;
        }
        if ($project_id == NULL) {
            throw New Exception('[FATAL] A project ID could not be found for study: ' . $accession . ' so we cannot drop chado.genotypes_<project_id>');
        }
        tpps_log('[INFO] PROJECT_ID: ' . $project_id);

        tpps_log('[INFO] Attempting to drop table chado.genotypes_' . $project_id . '...');
        $sql = "";
        $sql .= "drop table IF EXISTS chado.genotypes_" . $project_id . ";";
        chado_query($sql, []);
        

        tpps_log('[INFO] Removing records from chado.feature_genotype table...');
        $sql = "";
        $sql .= "delete from chado.feature_genotype where genotype_id in ";
        $sql .= "   (select distinct genotype_id from chado.genotype_call where project_id = (";
        $sql .= "       select project_id from chado.project_dbxref where dbxref_id = (";
        $sql .= "           select dbxref_id from chado.dbxref where accession = '" . $accession . "'";
        $sql .= "       )";
        $sql .= "   )";
        $sql .= ");";
        chado_query($sql, []);
        tpps_log('[INFO] Done.');

        tpps_log('[INFO] Removing records from chado.genotype table...');
        $sql = "";
        $sql .= "delete from chado.genotype where genotype_id in ";
        $sql .= "   (select distinct genotype_id from chado.genotype_call where project_id = (";
        $sql .= "       select project_id from chado.project_dbxref where dbxref_id = (";
        $sql .= "           select dbxref_id from chado.dbxref where accession = '" . $accession . "'";
        $sql .= "       )";
        $sql .= "   )";
        $sql .= ");";
        chado_query($sql, []);
        tpps_log('[INFO] Done.');

        tpps_log('[INFO] Removing records from chado.studies_with_markers table...');
        $sql = "";
        $sql .= "delete from chado.studies_with_markers where accession = '" . $accession . "'";
        chado_query($sql, []);
        tpps_log('[INFO] Done.');
        tpps_log('[INFO] Removing records from chado.markers_and_study_accession_per_individual_tree table...');
        $sql = "";
        $sql .= "delete from chado.markers_and_study_accession_per_individual_tree where accession = '" . $accession . "'";
        chado_query($sql, []);
        tpps_log('[INFO] Done.');

        tpps_log('[INFO] Removing records from chado.feature table...');
        $sql = "";
        $sql .= "delete from chado.feature where feature_id in (";
        $sql .= "   select distinct marker_id from chado.genotype_call where project_id = (";
        $sql .= "       select project_id from chado.project_dbxref where dbxref_id = (";
        $sql .= "           select dbxref_id from chado.dbxref where accession = '" . $accession . "'";
        $sql .= "       )";
        $sql .= "   )";
        $sql .= ")";
        $sql .= "or feature_id in (";
        $sql .= "   select distinct variant_id from chado.genotype_call where project_id = (";
        $sql .= "       select project_id from chado.project_dbxref where dbxref_id = (";
        $sql .= "           select dbxref_id from chado.dbxref where accession = '" . $accession . "'";
        $sql .= "       )";
        $sql .= "   )";
        $sql .= ");";
        chado_query($sql, []);
        tpps_log('[INFO] Done.');

        // Removed by Emily's new suggestions on 8/15/2024
        // $job->logMessage('[INFO] Removing records from chado.genotype_call table...');
        // $sql = "";
        // $sql .= "delete from chado.genotype_call where project_id = (";
        // $sql .= "   select project_id from chado.project_dbxref where dbxref_id = (";
        // $sql .= "       select dbxref_id from chado.dbxref where accession = '" . $accession . "'";
        // $sql .= "   )";
        // $sql .= ");";
        // chado_query($sql, []);
        // $job->logMessage('[INFO] Done.');        
        tpps_log('[STATUS] Job has been completed successfully without errors.');
        tpps_log("[INFO] Complete!");
        fclose($tpps_job_logger['log_file_handle']);
    }
    catch (Exception $e) {
        $transaction->rollback();
        //$job->logMessage('[ERROR] Job failed', array(), TRIPAL_ERROR);
        //$job->logMessage('[ERROR] Error message: @msg', array('@msg' => $e->getMessage()), TRIPAL_ERROR);
        //$job->logMessage("[ERROR] Trace: \n@trace", array('@trace' => $e->getTraceAsString()), TRIPAL_ERROR);

        // RISH 8/20/2024 - New code for job logs
        tpps_log('[ERROR] Job failed', [], TRIPAL_ERROR);
        tpps_log('[ERROR] Error message: @msg', ['@msg' => $e->getMessage()], TRIPAL_ERROR);
        tpps_log("[ERROR] Trace: \n!trace", ['!trace' => $e->getTraceAsString()], TRIPAL_ERROR);
    
        fclose($tpps_job_logger['log_file_handle']);
        watchdog_exception('tpps', $e);       
    }
}