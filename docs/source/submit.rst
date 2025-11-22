Data Submission
===============

Once the Input validation is completed for each of the 4 sets of fields, the data is saved in a persistent variable in the database, where it will wait until it is approved or rejected by an administrator. Both the user and administrator will be alerted once the submission has been completed, and again when the submission has been approved or rejected.

Upon approval, the data from the persistent variable is parsed, organized, and submitted according to the CHADO schema. TPPS makes sure not to overwrite existing entries in CHADO.

Persistent variables that TPPS creates will be removed from the database upon uninstallation.

