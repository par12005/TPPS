*********************************
Publication and Species Interface
*********************************

The first set of fields in TPPS is the publication and species interface, where users upload data about their publication and up to 5 studied species. The form fields and their properties are as follows:

* User Info: ``fieldset``

  * Primary Author: ``textfield`` - auto-populates with the name registered to their Tripal account. If the primary author is changed, autocomplete options are provided from the ``chado.contact`` table.
  * Organization: ``textfield`` - auto-populates with the organization registered to their Tripal account. If the organization is changed, autocomplete options are provided from the ``chado.contact`` table.

* Publication: ``fieldset``

  * Secondary Authors: ``fieldset``

     * Secondary Author **x**: ``textfield`` - autocomplete options from the ``chado.contact`` table
     * \>30 Secondary Authors: ``checkbox``
     * Secondary Authors file: ``managed_file`` - spreadsheet of secondary authors. This field is only visible if the '>30 Secondary Authors' checkbox is checked.

  * Publication status: ``select`` - options 'In Preparation or Submitted', 'In press', and 'Published'
  * Publication Year: ``select`` - options '1990' to '2018'
  * Publication Title: ``textfield``
  * Publication Abstract: ``textarea``
  * Publication Journal: ``textfield`` - autocomplete options from ``chado.pub`` table

* Tree Species: ``fieldset``

  * Species **x**: ``textfield`` -  autocomplete options from the ``chado.organism`` table
  * Up to 5 different species are allowed per submission

.. image:: ../../../screenshots/TPPS_author_species.png


