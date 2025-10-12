<?php

namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

/**
 * Creates a test class for the TPPS Project initialization process.
 *
 * This class is an extension of the TripalTestCase class, so it will be
 * included in phpunit tests and will be able to use the Tripal Test Suite.
 */
class ProjectInitTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * This method tests the tpps_init_project() function.
   *
   * Create an empty form state, pass it to tpps_init_project(), then ensure
   * that the accession number of the state itself matches that of the state's
   * saved_values front page. Also check that the accession begins with TGDR,
   * that there is a record for the submission in chado.dbxref, and that the
   * record's accession number matches that of the returned form state from
   * tpps_init_project().
   */
  public function testProjectInit() {
    $state = array();
    tpps_init_project($state);
    $this->assertTrue($state['accession'] == $state['saved_values']['frontpage']['accession']);
    $this->assertTrue(substr($state['accession'], 0, 4) == 'TGDR');
    $result = chado_select_record('dbxref', array('accession'), array('dbxref_id' => $state['dbxref_id']));
    $this->assertTrue((boolean) $result);
    $this->assertTrue($result[0]->accession == $state['accession']);
  }

}
