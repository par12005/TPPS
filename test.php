<?php

$path = '/var/www/html/sites/default/files/tpps_genotype/a.tar.gz';
$unzip_dir = '/var/www/html/sites/default/files/tpps_genotype/tmp';
$zip = new \PharData($path);
#$zip->decompress();
#$zip->extractTo($unzip_dir);
print_r("test\n");

