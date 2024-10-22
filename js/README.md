## JavaScript Debug Mode


### Overview

To debug/troubleshoot JS code we have "JS Debug Mode" which could be enabled
and disabled at admin's settings page (/admin/config/tpps/misc).
When "JS Debug Mode" enabled for the feature then log messages will be shown in
developer's console.

"JS Debug Mode" works only when special dog() function used to dump variables.
If developers uses 'console.log*()' then this messages always will be shown and
to disable them you need to edit code.


### Quick start

Just replace 'console.log(' with 'dog(':

```
var featureName = 'quickDebugMode';
// Was:
console.log('Got fid: ' + fid);
// Became:
dog('Got fid: ' + fid);
```


### Permanent feature name

Replace featureName with the name of current function (if necessary):
```
var featureName = 'jQuery.fn.fileColumnsChange';
```

Add new feature to the list of "JS Debug Mode Features":
1. Open /includes/common.inc file
2. Find 'function tpps_js_debug_mode_get_features_list()'
3. Add new item to the array.


### Feature Name Autodetection

For simple function names function dog() will be able to automatically detect
feature name using caller function name:
```
dog('Got fid: ' + fid);
```
When feature name detected incorrectly developer could explicitly spesify
feature name using 2nd argument of the function dog().

```
var featureName = 'jQuery.fn.fileColumnsChange';
dog('Got fid: ' + fid, featureName);
```

WARNIG:
Be sure to use the same feature name in JS code and at server side in PHP's
"JS Debug Mode Feature List".



### To dump JavaScript Object as a table:

var locations = {
  { 1, 2, 3},
  { 2, 3, 4}
}

dog('List of locations', locations);
// or with explicit feature name:
var featureName = 'jQuery.fn.fileColumnsChange';
dog('List of locations', locations, featureName);


Note: feeatureName is stil optional here.
