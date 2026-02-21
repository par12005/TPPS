/**
 * @file
 *
 * Block 28. TreeGenes Data Summary Widget (Splash).
 */
(function($, Drupal) {
  $(document).ready(function() {
  /* ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
    console.log('start');
    // Make an SVG Container.
    svgContainer = d3.select('#leaderboard');

    let data=Drupal.settings.custom_blocks;
    // Background rectangle.
    svgContainer
      .append('rect')
      .attr('x', 0)
      .attr('y', 0)
      .attr('width', 240)
      .attr('height', 320)
      .attr('fill', '#e9f9ef');

    //Top Caption Background rectangle
    //svgContainer
    //  .append('rect')
    //  .attr('x', 20)
    //  .attr('y', 5)
    //  .attr('width', 200)
    //  .attr('height', 20)
    //  .attr('fill', 'white');

    //Top Caption Text
    //var top_label = svgContainer
    //  .append('a')
    //  .attr('xlink:href', 'https://treegenesdb.org/seqsum')
    //  .append('text')
    //  .attr('x', 120)
    //  .attr('y', 15)
    //  .text('TREEGENES DATA SUMMARY')
    //  .attr('font-family', 'sans-serif')
    //  .attr('font-size', 11)
    //  .attr('text-anchor', 'middle')
    //  .attr('alignment-baseline', 'central');



    //SVG for top graph
    //var topgraph_svg = svgContainer.append('svg').attr('width', 220).attr('height',80).attr('x',10).attr('y',10);
    //var topgraph_background = topgraph_svg.append('rect').attr('x',0).attr('y',0).attr('width',220).attr('height',80).attr('fill', 'white');
    //var topgraph_width = 240,topgraph_height = 100;
    //var data = [0, 120, 500, 70];
    //var barHeight = 10;
    //var bar = topgraph_svg.selectAll('rect').data(data).enter().append('rect').attr('width', function(d) {  return d; }).attr('height', //barHeight - 1).attr('transform', function(d, i) {   return 'translate(0,' + i * barHeight + ')';  });

    var circle_phenotypes_back = svgContainer
      .append('circle')
      .attr('cx', 50)
      .attr('cy', 50)
      .attr('r', 35)
      .style('stroke', '#aad4c6')
      .attr('stroke-width', 4)
      .style('fill', 'white');

    var circle_phenotypes = svgContainer
      .append('circle')
      .attr('cx', 50)
      .attr('cy', 50)
      .attr('r', 30)
      .style('stroke', '#aad4c6')
      .attr('stroke-width', 1)
      .style('fill', 'white');

    var circle_phenotypes_text = svgContainer
      .append('a')
      .attr('xlink:href', '/seqsum')
      .append('text')
      .attr('x', 50)
      .attr('y', 45)
      .text(data.phenotypes)
      .attr('font-family', 'sans-serif')
      .attr('font-size', 14)
      .attr('text-anchor', 'middle')
      .attr('alignment-baseline', 'central');

    var circle_phenotypes_label = svgContainer
      .append('text')
      .attr('x', 50)
      .attr('y', 60)
      .text('PHENOTYPES')
      .attr('font-family', 'sans-serif')
      .attr('font-size', 7)
      .attr('text-anchor', 'middle')
      .attr('alignment-baseline', 'central');

    var circle_transcripts_back = svgContainer
      .append('circle')
      .attr('cx', 120)
      .attr('cy', 60)
      .attr('r', 35)
      .style('stroke', '#aad4c6')
      .attr('stroke-width', 4)
      .style('fill', 'white');

    var circle_transcripts = svgContainer
      .append('circle')
      .attr('cx', 120)
      .attr('cy', 60)
      .attr('r', 30)
      .style('stroke', '#aad4c6')
      .attr('stroke-width', 1)
      .style('fill', 'white');

    var circle_transcripts_text = svgContainer
      .append('a')
      .attr('xlink:href', '/seqsum')
      .append('text')
      .attr('x', 120)
      .attr('y', 55)
      .text(data.transcripts)
      .attr('font-family', 'sans-serif')
      .attr('font-size', 14)
      .attr('text-anchor', 'middle')
      .attr('alignment-baseline', 'central');

    var circle_transcripts_label = svgContainer
      .append('text')
      .attr('x', 120)
      .attr('y', 70)
      .text('TRANSCRIPTS')
      .attr('font-family', 'sans-serif')
      .attr('font-size', 7)
      .attr('text-anchor', 'middle')
      .attr('alignment-baseline', 'central');

    var circle_markers_back = svgContainer
      .append('circle')
      .attr('cx', 190)
      .attr('cy', 50)
      .attr('r', 35)
      .style('stroke', '#aad4c6')
      .attr('stroke-width', 4)
      .style('fill', 'white');

    var circle_markers = svgContainer
      .append('circle')
      .attr('cx', 190)
      .attr('cy', 50)
      .attr('r', 30)
      .style('stroke', '#aad4c6')
      .attr('stroke-width', 1)
      .style('fill', 'white');

    var circle_markers_text = svgContainer
      .append('a')
      .attr('xlink:href', '/seqsum')
      .append('text')
      .attr('x', 190)
      .attr('y', 45)
      .text(data.markers)
      .attr('font-family', 'sans-serif')
      .attr('font-size', 14)
      .attr('text-anchor','middle')
      .attr('alignment-baseline', 'central');

    var circle_markers_label = svgContainer
      .append('text')
      .attr('x', 190)
      .attr('y', 60)
      .text('MARKERS')
      .attr('font-family', 'sans-serif')
      .attr('font-size', 7)
      .attr('text-anchor', 'middle')
      .attr('alignment-baseline','central');

    //Leaf image
    //https://treegenesdb.org/sites/default/files/uploads/leaderboard-leaf.png
    svgContainer
      .append('svg:image')
      .attr('x', -15)
      .attr('y', 0)
      .attr('width', 400)
      .attr('height', 300)
      .attr('xlink:href', 'https://treegenesdb.org/sites/default/files/uploads/leaderboard-leaf.gif')
      .attr('transform', 'translate(0, 0) rotate(25)');

     //Draw the Circles
    var circle1_back = svgContainer
      .append('circle')
      .attr('cx', 50)
      .attr('cy', 220)
      .attr('r', 25)
      .style('stroke', '#aad4c6')
      .attr('stroke-width', 4)
      .style('fill', 'white');

    var circle1 = svgContainer
      .append('circle')
      .attr('cx', 50)
      .attr('cy', 220)
      .attr('r', 20)
      .style('stroke', '#aad4c6')
      .attr('stroke-width', 1)
      .style('fill', 'white')
      .on('mouseover', function(d, i) {
        // circle1_back.transition().duration('500').attr('r', 30);
      })
      .on('mouseout', function(d, i) {
        // circle1_back.transition().duration('500').attr('r', 25);
      });
    // var circle1_text = svgContainer
    //  .append('text')
    //  .attr('x', 40)
    //  .attr('y', 220)
    //  .text('$result_records_absolute_orders_count')
    //  .attr('font-family', 'sans-serif')
    //  .attr('font-size', 16);
    var circle1_text = svgContainer
      .append('a')
      .attr('xlink:href', '/species_directory/main')
      .append('text')
      .attr('x', 40)
      .attr('y', 220)
      .text(data.result_records_absolute_orders_count)
      .attr('font-family', 'sans-serif')
      .attr('font-size', 16);

    var circle1_label = svgContainer
      .append('text')
      .attr('x', 33)
      .attr('y', 230)
      .text('ORDERS')
      .attr('font-family', 'sans-serif')
      .attr('font-size', 8);

    var circle2_back = svgContainer
      .append('circle')
      .attr('cx', 95)
      .attr('cy', 185)
      .attr('r', 30)
      .style('stroke', '#aad4c6')
      .attr('stroke-width', 7)
      .style('fill', 'white')
      .on('mouseover', function(d, i) {
        // d3.select(this).transition().duration('500').attr('r', 35);
      })
      .on('mouseout', function(d, i) {
        // d3.select(this).transition().duration('500').attr('r', 30);
      });
    var circle2 = svgContainer
      .append('circle')
      .attr('cx', 95)
      .attr('cy', 185)
      .attr('r', 24)
      .style('stroke', '#aad4c6')
      .attr('stroke-width',1)
      .style('fill', 'white');

    // var circle2_text = svgContainer
      // .append('text')
      // .attr('x', 80)
      // .attr('y', 185)
      // .text(data.result_records_absolute_genus_count)
      // .attr('font-family', 'sans-serif')
      // .attr('font-size', 16);
    var circle2_text = svgContainer
      .append('a')
      .attr('xlink:href', '/species_directory/main')
      .append('text')
      .attr('x', 80)
      .attr('y', 185)
      .text(data.result_records_absolute_genus_count)
      .attr('font-family', 'sans-serif')
      .attr('font-size', 16);

    var circle2_label = svgContainer
      .append('text')
      .attr('x', 80)
      .attr('y', 195)
      .text('GENUS')
      .attr('font-family', 'sans-serif')
      .attr('font-size', 8);

    var circle3_back = svgContainer
      .append('circle')
      .attr('cx', 130)
      .attr('cy', 235)
      .attr('r', 34)
      .style('stroke', '#aad4c6')
      .attr('stroke-width', 7)
      .style('fill', 'white')
      .on('mouseover', function(d,i) {
        // d3.select(this).transition().duration('500').attr('r', 39);
      })
      .on('mouseout', function(d,i) {
        // d3.select(this).transition().duration('500').attr('r', 34);
      });
    var circle3 = svgContainer
      .append('circle')
      .attr('cx', 130)
      .attr('cy', 235)
      .attr('r', 28)
      .style('stroke', '#aad4c6')
      .attr('stroke-width', 1)
      .style('fill', 'white');

    // var circle3_text = svgContainer
      // .append('text')
      // .attr('x',113)
      // .attr('y',232)
      // .text(data.result_records_absolute_species_count)
      // .attr('font-family','sans-serif')
      // .attr('font-size',16);

    var circle3_text = svgContainer
      .append('a')
      .attr('xlink:href', '/species_directory/main')
      .append('text')
      .attr('x', 113)
      .attr('y', 232)
      .text(data.result_records_absolute_species_count)
      .attr('font-family', 'sans-serif')
      .attr('font-size', 16);

    var circle3_label = svgContainer
      .append('text')
      .attr('x',111)
      .attr('y',246)
      .text('SPECIES')
      .attr('font-family','sans-serif')
      .attr('font-size',9);

    var circle4_back = svgContainer
      .append('circle')
      .attr('cx', 180)
      .attr('cy', 180)
      .attr('r', 34)
      .style('stroke', '#aad4c6')
      .attr('stroke-width',7)
      .style('fill', 'white')
      .on('mouseover', function(d, i) {
        // d3.select(this).transition().duration('500').attr('r', 39);
      })
      .on('mouseout', function(d, i) {
        // d3.select(this).transition().duration('500').attr('r', 34);
      });
    var circle4 = svgContainer
      .append('circle')
      .attr('cx', 180)
      .attr('cy', 180)
      .attr('r', 28)
      .style('stroke', '#aad4c6')
      .attr('stroke-width',1)
      .style('fill', 'white');

    var circle4_text = svgContainer
      .append('text')
      .attr('x', 180)
      .attr('y', 175)
      .text(data.genomes)
      .attr('font-family', 'sans-serif')
      .attr('font-size', 16)
      .attr('text-anchor', 'middle')
      .attr('alignment-baseline', 'central');

    var circle4_label = svgContainer
      .append('text')
      .attr('x', 180)
      .attr('y', 190)
      .text('GENOMES')
      .attr('font-family', 'sans-serif')
      .attr('font-size', 9)
      .attr('text-anchor', 'middle')
      .attr('alignment-baseline', 'central');
    //svgContainer
      //.append('rect')
      //.attr('x', 50)
      //.attr('y', 20)
      //.attr('width', 100)
      //.attr('height', 50)
      //.attr('fill', 'green');
    console.log('end');

  /* ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
  });
})(jQuery, Drupal);
