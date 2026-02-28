<?php

/**
 * @file
 * Contains 'GoogleScholar' class.
 *
 * Usage:
 *  $gs = new GoogleScholar($doi);
 *  $gs->request();
 *  $gs->getJornal();
 *  $gs->update();
 *
 * Workflow
 * 1. Set DOI.
 * 2. Get response using RestAPI.
 * 3. Store in DB.
 * 4. Parse.
 *
 * n. Update if stored pages is outdated.
 */

/**
 * Gets publication info by DOI form Google Scholar.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class GoogleScholar {


}
