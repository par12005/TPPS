<?php
namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use stdClass;
use ZipArchive;

class FileUtilsTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Tests the tpps_get_path_extension() function.
   */
  public function testGetExtension() {
    $file1 = DRUPAL_ROOT . '/' . drupal_get_path('module', 'tpps') . '/tests/test_files/tpps_accession_test_1.xlsx';
    $file2 = DRUPAL_ROOT . '/' . drupal_get_path('module', 'tpps') . '/tests/test_files/tpps_accession_test.csv';

    $this->assertTrue(tpps_get_path_extension($file1) === 'xlsx');
    $this->assertTrue(tpps_get_path_extension($file2) === 'csv');
  }

  /**
   * Tests the tpps_xlsx_get_dimension() function.
   *
   * Verify that the dimensions of the two accession .xlsx files are correct
   * according to tpps_xlsx_get_dimension().
   */
  public function testGetDimensions() {
    $dir = drupal_realpath(TPPS_TEMP_XLSX);
    $path1 = DRUPAL_ROOT . '/' . drupal_get_path('module', 'tpps') . '/tests/test_files/tpps_accession_test_1.xlsx';
    $path2 = DRUPAL_ROOT . '/' . drupal_get_path('module', 'tpps') . '/tests/test_files/tpps_accession_test_2.xlsx';

    // Test first file dimension.
    $zip = new ZipArchive();
    $zip->open($path1);
    $zip->extractTo($dir);
    $data_location = $dir . '/xl/worksheets/sheet1.xml';
    $this->assertTrue(tpps_xlsx_get_dimension($data_location) === 'A1:D7');
  
    // Test second file dimension.
    $zip = new ZipArchive();
    $zip->open($path2);
    $zip->extractTo($dir);
    $data_location = $dir . '/xl/worksheets/sheet1.xml';
    $this->assertTrue(tpps_xlsx_get_dimension($data_location) === 'A1:D9');
  }

  /**
   * Tests the tpps_convert_colname() function.
   */
  public function testConvertColname() {
    $this->assertEquals(0, tpps_convert_colname('A'));
    $this->assertEquals(1, tpps_convert_colname('B'));
    $this->assertEquals(26, tpps_convert_colname('AA'));
    $this->assertEquals(27, tpps_convert_colname('AB'));
    $this->assertEquals(52, tpps_convert_colname('BA'));
  }

  /**
   * Tests the tpps_increment_hex() function.
   */
  public function testIncrementHex() {
    $this->assertEquals('B', pack('H*', tpps_increment_hex(unpack('H*', 'A')[1])));
    $this->assertEquals('AB', pack('H*', tpps_increment_hex(unpack('H*', 'AA')[1])));
    $this->assertEquals('AA', pack('H*', tpps_increment_hex(unpack('H*', 'Z')[1])));
    $this->assertEquals('BA', pack('H*', tpps_increment_hex(unpack('H*', 'AZ')[1])));
    $this->assertEquals('AAAA', pack('H*', tpps_increment_hex(unpack('H*', 'ZZZ')[1])));
  }

  /**
   * Creates a Drupal file object from a test file path.
   *
   * @param string $path
   *   The test file path.
   *
   * @return stdClass
   *   The resulting Drupal file object.
   */
  public function initializeTestFile($path) {
    return file_save((object)array(
      'filename' => basename($path),
      'uri' => $path,
      'status' => 0,
      'filemime' => file_get_mimetype($path),
    ));
  }

}
