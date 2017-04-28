TGDR

Directory Location:
Server: 155.37.254.147
Path:   /var/www/Drupal/sites/all/modules/CustomModule/custom_module
All contents are in this directory, currently no need to look elsewhere.

Issues:

Currently having an issue with getting class of select element in Drupal API to apply to title of select, description of select, and select element itself.  The class is currently only applied to the element itself.
Need to ask Stephen Ficklin or Lacey about this.
Current work around is as follows (in custom_module.js):

    jQuery('.form-item-commonGardenSelect').addClass('commonGardenClass');
    jQuery('.form-item-plantationSelect').addClass('plantationClass');
    jQuery('.form-item-natPopSelect').addClass('natPopClass');
