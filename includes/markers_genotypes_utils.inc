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
        $job->logMessage('[INFO] Removing records from chado.genotype table...');
        $sql = "";
        $sql .= "delete from chado.genotype where genotype_id in ";
        $sql .= "   (select distinct genotype_id from chado.genotype_call where project_id = (";
        $sql .= "       select project_id from project_dbxref where dbxref_id = (";
        $sql .= "           select dbxref_id from dbxref where accession = '" . $accession . "'";
        $sql .= "       )";
        $sql .= "   )";
        $sql .= ");";
        chado_query($sql, []);
        $job->logMessage('[INFO] Done.');

        $job->logMessage('[INFO] Removing records from chado.feature table...');
        $sql = "";
        $sql .= "delete from chado.feature where feature_id in (";
        $sql .= "   select distinct marker_id from chado.genotype_call where project_id = (";
        $sql .= "       select project_id from project_dbxref where dbxref_id = (";
        $sql .= "           select dbxref_id from dbxref where accession = '" . $accession . "'";
        $sql .= "       )";
        $sql .= "   )";
        $sql .= ")";
        $sql .= "or feature_id in (";
        $sql .= "   select distinct variant_id from chado.genotype_call where project_id = (";
        $sql .= "       select project_id from project_dbxref where dbxref_id = (";
        $sql .= "           select dbxref_id from dbxref where accession = '" . $accession . "'";
        $sql .= "       )";
        $sql .= "   )";
        $sql .= ");";
        chado_query($sql, []);
        $job->logMessage('[INFO] Done.');

        $job->logMessage('[INFO] Removing records from chado.genotype_call table...');
        $sql = "";
        $sql .= "delete from chado.genotype_call where project_id = (";
        $sql .= "   select project_id from project_dbxref where dbxref_id = (";
        $sql .= "       select dbxref_id from dbxref where accession = '" . $accession . "'";
        $sql .= "   )";
        $sql .= ");";
        chado_query($sql, []);
        $job->logMessage('[INFO] Done.');        
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