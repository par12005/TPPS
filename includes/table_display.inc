<?php

function tpps_table_display($state){
    
    $accession = $state['saved_values']['fourthPage']['accession'];
    $display = "<div class='tpps_complete_submission'><table id='tpps_$accession' style='width:-webkit-fill-available' border='1'><thead>";
    //$display .= "<tr><th>Field</th><th>Value</th>";
    $display .= "</thead><tbody>";
    
    $flat = tpps_flatten($state);
    
    foreach ($flat as $key => $val){
        if ($key[0] !== '#'){
            $key_str = filter_xss(check_plain($key));
            $val_str = filter_xss(check_plain($val));

            if (preg_match('/File/', $key) and ($file_name = file_load($val)->filename)){
                //create links to files
                $file_url = check_plain(file_create_url(file_load($val)->uri));
                $val_str = "<a href='$file_url' target='blank'>$file_name</a>";
            }
            elseif (preg_match('/Month/', $key)){
                //skip experiment start/end month
                continue;
            }
            elseif (preg_match('/^(.*)Year/', $key, $matches)){
                //combine experiment start/end month + year
                if (isset($flat[$matches[1] . 'Month'])){
                    $key_str = $matches[1] . 'Date';
                    $val_str = "{$flat[$matches[1] . 'Month']} $val_str";
                }
                else {
                    continue;
                }
            }
            elseif (preg_match('/Study Location/', $key) and isset($flat['#location_type']) and $flat['#location_type']){
                //add map capability to location
                if ($flat['#location_type'] == '2'){
                    $query = $val_str;
                }
                elseif ($flat['#location_type'] != '2' and ($standard = tpps_standard_coord($val_str))){
                    $query = $standard;
                }
                $val_str = "<a id=\"sidebar_map\" style=\"cursor:pointer\">$val_str</a>";
                if (isset($query)){
                    $map = "<iframe frameborder=\\\"0\\\" style=\\\"border:0;width:100%;height:100%\\\" src=\\\"https://www.google.com/maps?q=$query&output=embed&key=AIzaSyDkeQ6KN6HEBxrIoiSCrCHFhIbipycqouY&z=5\\\" allowfullscreen> </iframe></div>";
                    $js = "jQuery(document).ready(function ($) {
                        jQuery(\"#sidebar_map\").click(function() {
                            jQuery(\"#sidebar_map_wrapper\").remove();
                            jQuery(\"#sidebar_map\").after( \"<br><div id=\\\"sidebar_map_wrapper\\\">$map</div>\" );
                        });
                    });";
                    drupal_add_js($js, 'inline');
                }
            }

            $display .= "<tr><td>$key_str</td><td>$val_str</td></tr>";
        }
    }
    
    $display .= "</tbody></table></div><br>";
    
    return $display;
}
