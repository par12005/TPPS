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
    $accession = trim($accession);
    if($accession == "") {
        throw new Exception('Accession must be valid, empty accession was provided');
    }
    $job->logMessage('[INFO] Performing removal of markers and genotypes on study: ' . $accession);
    $job->setInterval(1);

    $transaction = db_transaction();
    try {

        // Get Project ID
        $job->logMessage('[INFO] Finding project_id...');
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
        $job->logMessage('[INFO] PROJECT_ID: ' . $project_id);

        $job->logMessage('[INFO] Attempting to drop table chado.genotypes_' . $project_id . '...');
        $sql = "";
        $sql .= "drop table IF EXISTS chado.genotypes_" . $project_id . ";";
        chado_query($sql, []);
        

        $job->logMessage('[INFO] Removing records from chado.feature_genotype table...');
        $sql = "";
        $sql .= "delete from chado.feature_genotype where genotype_id in ";
        $sql .= "   (select distinct genotype_id from chado.genotype_call where project_id = (";
        $sql .= "       select project_id from chado.project_dbxref where dbxref_id = (";
        $sql .= "           select dbxref_id from chado.dbxref where accession = '" . $accession . "'";
        $sql .= "       )";
        $sql .= "   )";
        $sql .= ");";
        chado_query($sql, []);
        $job->logMessage('[INFO] Done.');

        $job->logMessage('[INFO] Removing records from chado.genotype table...');
        $sql = "";
        $sql .= "delete from chado.genotype where genotype_id in ";
        $sql .= "   (select distinct genotype_id from chado.genotype_call where project_id = (";
        $sql .= "       select project_id from chado.project_dbxref where dbxref_id = (";
        $sql .= "           select dbxref_id from chado.dbxref where accession = '" . $accession . "'";
        $sql .= "       )";
        $sql .= "   )";
        $sql .= ");";
        chado_query($sql, []);
        $job->logMessage('[INFO] Done.');

        $job->logMessage('[INFO] Removing records from chado.studies_with_markers table...');
        $sql = "";
        $sql .= "delete from chado.studies_with_markers where accession = '" . $accession . "'";
        chado_query($sql, []);
        $job->logMessage('[INFO] Done.');
        $job->logMessage('[INFO] Removing records from chado.markers_and_study_accession_per_individual_tree table...');
        $sql = "";
        $sql .= "delete from chado.markers_and_study_accession_per_individual_tree where accession = '" . $accession . "'";
        chado_query($sql, []);
        $job->logMessage('[INFO] Done.');

        $job->logMessage('[INFO] Removing records from chado.feature table...');
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
        $job->logMessage('[INFO] Done.');

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
        $job->logMessage('[STATUS] Job has been completed successfully without errors.');
    }
    catch (Exception $e) {
        $transaction->rollback();
        $job->logMessage('[ERROR] Job failed', array(), TRIPAL_ERROR);
        $job->logMessage('[ERROR] Error message: @msg', array('@msg' => $e->getMessage()), TRIPAL_ERROR);
        $job->logMessage("[ERROR] Trace: \n@trace", array('@trace' => $e->getTraceAsString()), TRIPAL_ERROR); 
        watchdog_exception('tpps', $e);       
    }
}