(function ($) {
  Drupal.behaviors.tppsPage4 = {
    attach: function (context, settings) {
      // TPPS Form Page 4 Common.
      // Attach event handlers only once.
      $('form[id^=tppsc-main]', context).once('tppsPage4', function() {
        let $form = $(this);
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      });
      const featureName = 'Drupal.behaviors.tppsPage4';
      const organismNumber = Drupal.settings.tpps.organismNumber;
      // 'SynonymField' object.
      // @TODO Minor. Covert into separate and more common class to reuse it.
      const SynonymField = {
        // Common.
        // Value of the 'name' attribute. Name of the form element in DOM.
        'name' : '',

        // Form element (select element of the DOM).
        'element' : '',

        /**
         * Gets synonym field name.
         *
         * @param int organismId
         *   Organism Id.
         *   Note: Total number of organisms was set on Page 1 and fixed now.
         *   We loop organisms to process all fields on page.
         * @param int phenotypeId'
         *   Each organism (species) could have multiple phenotypes and their
         *   number could be changed on current page. We loop them to process
         *   all phenotype synonym fields on page.
         *
         * @return string
         *   Returns name of the synonym field in DOM.
         *   E.g., "organism-1[phenotype][phenotypes-meta][1][synonym_id]".
         *       organismId ^                 phenotypeId ^
         *   When SynonymField.element property set then we use form field's
         *   'name' attribute. When it's empty we build string using provided
         *   arguments.
         */
        'getName' : function(organismId = '', phenotypeId = '') {
          // Get previously stored name.
          if (typeof this.name != 'undefined' && this.name.length) {
            return this.name;
          }
          // Build name.
          if (typeof this.element != 'undefined' && this.element.length) {
            dog('getName: Used this.element', featureName);
            organismId = this.getOrganismId();
            phenotypeId = this.getPhenotypeId();
          }
          if (
            typeof organismId != 'undefined' && organismId != ''
            && typeof phenotypeId != 'undefined' && phenotypeId != ''
          ) {
            return 'organism-' + organismId + this.namePrefix
              + phenotypeId + this.nameSuffix;
          }
          else {
            dog('getName: not found.', featureName);
          }
        },

        /**
         * Sets field's form element.
         *
         * Stores form element into 'element' property.
         *
         * @param object element
         *   Form field element. Used to get fields name and then parse it to
         *   get Organism Id and Phenotype Id.
         */
        'setElement' : function(element) {
          if (typeof element != 'undefined' && element.length) {
            this.element = element;
            this.setName(element);
          }
          else {
            dog('setElement: empty element.', featureName);
            dog(element, featureName);
          }
          return this;
        },

        /**
         * Sets fields name.
         *
         * @param object element
         *   Form field element. Used to get fields name and then parse it to
         *   get Organism Id and Phenotype Id.
         */
        'setName' : function(element) {
          if (typeof this.element != 'undefined' && this.element.length) {
            this.name = $(this.element).attr('name');
          }
          else if (typeof element != 'undefined') {
            this.name = $(element).attr('name');
          }
        },

        /**
         * Gets Organism Id.
         *
         * Organism Id is an ordinal number of species on page.
         *
         * @param object element
         *   Optional form field element. When it's not empty we use it to get
         *   Organism Id from the field's name attribute. When it's empty we
         *   use previously filled SynonymField.name property.
         *
         * @return int
         *   Retuns Organism Id or 'undefined' when argument 'element' is empty
         *   or properties SynonymField.element and SynonymField.name are
         *   also empty.
         */
        'getOrganismId' : function(element = '') {
          if (typeof this.element != 'undefined' && this.element.length) {
            this.setElement(element);
          }
          if (typeof this.name != 'undefined' && this.name.length) {
            let regex = new RegExp(
              this.namePrefix.replace(/\]/g, '\\]').replace(/\[/g, '\\[')
              + "\\d+"
              + this.nameSuffix.replace(/\]/g, '\\]').replace(/\[/g, '\\[')
            );
            return parseInt(
              this.name.replace('organism-', '').replace(regex, ''), 10
            );
          }
          else {
            dog('getOrganismId: empty this.name', featureName);
          }
        },

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Custom.

        // Prefix of the the field's name. Used to build field name and parse
        // field name to get Organism Id and Phenotype Id.
        'namePrefix' : '[phenotype][phenotypes-meta][',

        // Suffix of the the field's name. Used to build field name and parse
        // field name to get Organism Id and Phenotype Id.
        'nameSuffix' : '][synonym_id]',
        /**
         * Gets Phenotype Id.
         *
         * Pheonoty Id is an ordinal number of phenotype per species on page.
         *
         * @param object element
         *   Optional form field element. When it's not empty we use it to get
         *   phenotype id from the field's name attribute. When it's empty we
         *   use previously filled SynonymField.name property.
         *
         * @return int
         *   Retuns phenotype Id or 'undefined' when argument 'element' is empty
         *   or properties SynonymField.element and SynonymField.name are
         *   also empty.
         */
        'getPhenotypeId' : function(element = '') {
          if (typeof this.element != 'undefined' && this.element.length) {
            this.setElement(element);
          }
          if (typeof this.name != 'undefined' && this.name.length) {
            return parseInt(this.name
              .replace(/organism-\d+/, '')
              .replace(this.namePrefix, '')
              .replace(this.nameSuffix, ''), 10
            );
          }
          else {
            dog('getPhenotypeId: empty this.name', featureName);
          }
        },

        /**
         * Disables sibling Unit dropdown menu.
         *
         * When Phenotype/Synonym field is changes we do AJAX-request to update
         * list of related units in dropdown menu and it could take some time.
         * To improve UX we block related unit selection field while
         * AJAX-request is processing. In case of success existing fields will
         * be replaced with new one received from server so we don't need to
         * re-enable those fields.
         *
         * @param int organismId
         *   Organism Id.
         *   Note: Total number of organisms was set on Page 1 and fixed now.
         *   We loop organisms to process all fields on page.
         * @param int phenotypeId'
         *   Each organism (species) could have multiple phenotypes and their
         *   number could be changed on current page. We loop them to process
         *   all phenotype synonym fields on page.
         */
        'disableUnitField' : function(organismId = '', phenotypeId = '') {
          if (typeof this.element != 'undefined' && this.element.length) {
            organismId = this.getOrganismId();
            phenotypeId = this.getPhenotypeId();
          }
          if (
            typeof organismId != 'undefined'
            && typeof phenotypeId != 'undefined'
          ) {
            let unitFieldName = 'organism-' + organismId + this.namePrefix
              + phenotypeId + '][unit]';
            dog('Unit Field Name: ' + unitFieldName, featureName);
            $('select[name="' + unitFieldName + '"]')
              //.css({'border' : '1px solid red'})
              .prop('disabled', true);
          }
          else {
            dog("Unit field wasn't disabled.", featureName);
          }
          return this;
        },
      };

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Loop organisms and phenotypes on page to find all Phenotype/Synonym
      // fields and block related Unit dropdown menu when Phenotype fields
      // are changed.
      for (let organismId = 1; organismId <= organismNumber; organismId++) {
        // @TODO Minor. Label field to process only once.
        // Get actual number of phenotypes on page from DOM.
        // Note: number of phenotypes could be different per species.
        let phenotypeNumber = parseInt($('input[name="organism-'
          + organismId + '[phenotype][phenotypes-meta][number]').val(), 10);
        dog('Number of phenotypes for Organism #' + organismId + ': '
          + phenotypeNumber, featureName);

        for (let phenotypeId = 1; phenotypeId <= phenotypeNumber; phenotypeId++) {
          let name = SynonymField.getName(organismId, phenotypeId);
          var $field = $('select[name="' + name + '"]', context);
          if ($field.length == 0) {
            continue;
          }
          $field.on('change', function(e) {
            SynonymField
              .setElement(this)
              .disableUnitField();
          });
        }
      }
    }
  }
})(jQuery);
