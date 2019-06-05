<?php
namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

class ProjectInitTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  public function testProjectInit() {
    $state = array();
    tpps_init_project($state);
    $this->assertTrue($state['accession'] == $state['saved_values']['frontpage']['accession']);
    $this->assertTrue(substr($state['accession'], 0, 4) == 'TGDR');
    $result = chado_select_record('dbxref', array('accession'), array('dbxref_id' => $state['dbxref_id']));
    $this->assertTrue((boolean)$result);
    $this->assertTrue($result[0]->accession == $state['accession']);
  }
}
