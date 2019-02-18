*********************************
Submission Approval and Rejection
*********************************

Once a submission is completed, it is not available to the public until it has been approved by an administrator. This can be done from the TPPS admin approval panel, which can be accessed in one of two ways:

1. Click the link in the alert sent to the site admin email
2. Navigate to ``http://<site domain>/<Drupal root>/tpps-admin-panel?accession=<accession number>``

Once you are on the submission approval panel, you can either fill out the rejection reason field and click ``Reject`` to reject the submission, or you can check the ``This submission has been reviewed and approved`` box, and click ``Approve`` to approve it.

If the submission is approved the user will recieve a notification via email, and the data that was not added through file uploads will be added to chado. The remaining data, which was added through file uploads, will be added to chado later during a tripal job. This data can take a long time to add, which is why it is added in a tripal job rather than immediately after approval.

If the submission is rejected, the user will recieve a notification via email, and the submission will move from the ``completed`` state back to an ``incomplete`` submission. That way, the user can see the comments from the administrator, and can make appropriate changes to their submission. 
