<?php
  $vcf_location = '/isg/treegenes/curation/TGDR2294/Shu_2024.v3.vcf.gz';
  $cmd_calls = "bcftools query -f '[%SAMPLE\\t%ID\\t%REF\\t%ALT\\t%GT\\n]' \"$vcf_location\" | head -n 10";
  //$cmd_calls = "bcftools query -f \"$vcf_location\" | head -n 10";
  // $cmd_calls = escapeshellcmd($cmd_calls);
  echo $cmd_calls . "\n";
  $output = shell_exec($cmd_calls);
  echo $output;
  //exec($cmd_calls, $output_lines);
  echo $output_lines;
  // passthru($cmd_calls, $return_var);
  // print_r($output_lines);
?>