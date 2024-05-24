<?php

namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

/**
 * Creates a test class for TPPS Submission functions.
 *
 * This class is an extension of the TripalTestCase class, so it will be
 * included in phpunit tests and will be able to use the Tripal Test Suite.
 */
class SubmissionsTest extends TripalTestCase {
  use DBTransaction;

  /**
   * Helper function for TPPS Submission tests.
   *
   * Logs in as the superuser, initializes an empty project, and creates the
   * corresponding submission.
   */
  public function loginAndCreateEmptySubmission() {
    $this->actingAs(1);

    global $user;
    $state = array();

    // Initialize project, create submission.
    tpps_init_project($state);
    tpps_create_submission($state, $user->uid);

    return $state;
  }

  /**
   * This method tests the tpps_create_submission() function.
   *
   * Checks that the tpps_create_submission() function properly creates the
   * submission record in tpps_submission, assigns the 'Incomplete' submission
   * status, and creates the 'created' timestamp.
   */
  public function testCreateSubmission() {
    $state = $this->loginAndCreateEmptySubmission();
    global $user;

    // Load submission info from database.
    $query = db_select('tpps_submission', 't')
      ->fields('t')
      ->condition('accession', $state['accession'])
      ->range(0, 1)
      ->execute();
    $result = $query->fetchObject();

    // Verify submission status and uid.
    $this->assertEquals(TPPS_SUBMISSION_STATUS_INCOMPLETE, $result->status);
    $this->assertEquals($user->uid, $result->uid);

    // Verify created timestamp exists and is earlier than current time.
    $state = unserialize($result->submission_state);
    $this->assertNotEmpty($state['created']);
    $this->assertLessThanOrEqual(time(), $state['created']);
  }

  /**
   * This method tests the tpps_load_submission() function.
   *
   * Checks that tpps_load_submission() returns null when accession is empty
   * or accession doesn't match anything in the tpps_submission table.
   */
  public function testLoadSubmissionFakeAccession() {
    $result = tpps_load_submission('fake_accession_this_should_not_match_anything');
    $this->assertEmpty($result);
    $result = tpps_load_submission('');
    $this->assertEmpty($result);
  }

  /**
   * This method tests the tpps_load_submission() function.
   *
   * Checks that tpps_load_submission() retrieves the correct submission state
   * when the $state param is set to TRUE, and retrieves the correct
   * tpps_submission database record when the $state param is set to FALSE.
   */
  public function testLoadSubmission() {
    $state = $this->loginAndCreateEmptySubmission();

    $result = tpps_load_submission($state['accession']);
    $this->assertEquals($state, $result);
    $result = tpps_load_submission($state['accession'], FALSE);
    $this->assertNotEquals($state, $result);
    $this->assertEquals($state, unserialize($result->submission_state));
  }

  /**
   * This method tests the tpps_update_submission() function.
   *
   * Checks that tpps_update_submission() assigns the 'updated' timestamp each
   * time it is called, updates the content of the submission state, and
   * updates the submission status in the db record when applicable.
   */
  public function testUpdateSubmission() {
    $state = $this->loginAndCreateEmptySubmission();
    $accession = $state['accession'];
    $submission = new Submission($accession);

    $new_item = [
      'title' => 'TPPS test title',
    ];
    $submission->state['saved_values'][TPPS_PAGE_1]['publication'] = $new_item;
    $submission->save();

    $submission->load();
    $this->assertNotEmpty($submission->state['updated']);
    $this->assertNotEmpty($submission->state['saved_values'][TPPS_PAGE_1]['publication']);
    $this->assertEquals(
      $submission->state['saved_values'][TPPS_PAGE_1]['publication'],
      $new_item
    );
    $updated = $submission->state['updated'];
    sleep(1);

    $submission->save();

    $submisison->load();
    $this->assertGreaterThan($updated, $submission->state['updated']);

    $submission->state['status'] = TPPS_SUBMISSION_STATUS_APPROVED;
    $submission->save();
    $this->assertEquals(TPPS_SUBMISSION_STATUS_APPROVED, $submission->status);
  }

  /**
   * This method tests the Submission::delete() method.
   *
   * Checks that Submission::delete() removes the submission from the
   * 'tpps_submission' table. For non-TPPSc submissions and submissions that do
   * not use an old TGDR number, checks that also the submission removed
   * from the chado.dbxref table.
   *
   * @TODO Check if files are removed.
   * @TODO Check purge() method.
   */
  public function testDeleteSubmission() {
    $state = $this->loginAndCreateEmptySubmission();
    $dbx = $state['dbxref_id'];
    $accession = $state['accession'];

    $this->assertNotEmpty(tpps_load_submission($accession));

    $submission = new Submission($accession);
    $submission->delete();

    $submission = new Submission($accession);
    $this->assertEmpty($submission->doesExist());

    $query = db_select('chado.dbxref', 'dbx')
      ->fields('dbx')
      ->condition('dbxref_id', $dbx)
      ->execute();
    $this->assertEmpty($query->fetchAll());
  }

}
