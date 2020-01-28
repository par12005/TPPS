<?php
namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use stdClass;
use ZipArchive;

class FileUtilsTest extends TripalTestCase {
  use DBTransaction;

  /**
   * Constructs a test case with the given name and creates some file paths.
   *
   * @param string $name
   * @param array  $data
   * @param string $dataName
   */
  function __construct($name = null, array $data = [], $dataName = '') {
    parent::__construct($name, $data, $dataName);
    $this->path1 = DRUPAL_ROOT . '/' . drupal_get_path('module', 'tpps') . '/tests/test_files/tpps_accession_test_1.xlsx';
    $this->path2 = DRUPAL_ROOT . '/' . drupal_get_path('module', 'tpps') . '/tests/test_files/tpps_accession_test_2.xlsx';
    $this->path3 = DRUPAL_ROOT . '/' . drupal_get_path('module', 'tpps') . '/tests/test_files/tpps_accession_test.csv';
  }

  /**
   * Tests the tpps_get_path_extension() function.
   */
  public function testGetExtension() {
    $this->assertTrue(tpps_get_path_extension($this->path1) === 'xlsx');
    $this->assertTrue(tpps_get_path_extension($this->path3) === 'csv');
  }

  /**
   * Tests the tpps_xlsx_get_dimension() function.
   *
   * Verify that the dimensions of the two accession .xlsx files are correct
   * according to tpps_xlsx_get_dimension().
   */
  public function testGetDimensions() {
    $dir = drupal_realpath(TPPS_TEMP_XLSX);

    // Test first file dimension.
    $zip = new ZipArchive();
    $zip->open($this->path1);
    $zip->extractTo($dir);
    $data_location = $dir . '/xl/worksheets/sheet1.xml';
    $this->assertTrue(tpps_xlsx_get_dimension($data_location) === 'A1:D7');
  
    // Test second file dimension.
    $zip = new ZipArchive();
    $zip->open($this->path2);
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
   * Tests the tpps_file_len() function.
   *
   * This function needs to work for both xlsx and csv files.
   */
  public function testFileLength() {
    $file1 = $this->initializeTestFile($this->path1);
    $this->assertEquals(6, tpps_file_len($file1->fid));
    
    $file2 = $this->initializeTestFile($this->path3);
    $this->assertEquals(2, tpps_file_len($file2->fid));
  }

  /**
   * Tests the tpps_file_width() function.
   */
  public function testFileWidth() {
    $file1 = $this->initializeTestFile($this->path1);
    $this->assertEquals(4, tpps_file_width($file1->fid));

    $file2 = $this->initializeTestFile($this->path2);
    $this->assertEquals(4, tpps_file_width($file2->fid));

    $file3 = $this->initializeTestFile($this->path3);
    $this->assertEquals(26, tpps_file_width($file3->fid));
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
