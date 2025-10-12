import argparse
import shutil
import subprocess
import tempfile
from pathlib import Path
import pandas as pd
import re
import numpy as np
from Bio import SeqIO
from datetime import datetime
import hashlib
import gzip
import os

# Coded by Meghan Myles (TreeGenesDB)

def parse_args():
    parser = argparse.ArgumentParser(description="Generate VCF from SNP assay data.")

    parser.add_argument("--assay-path", required=True)
    parser.add_argument("--assay-design-path", required=True)
    parser.add_argument("--species", required=True)
    parser.add_argument("--four-letter-code", required=True)
    parser.add_argument("--assembly-version", required=True)
    parser.add_argument("--assay-design-snp-name-col", required=True)
    parser.add_argument("--assay-design-snp-chrom-col", required=True)
    parser.add_argument("--assay-design-snp-base-pos-col", required=True)
    parser.add_argument("--assay-design-snp-flank-col", required=True)
    parser.add_argument("--assay-design-qual-col", required=True)
    parser.add_argument("--assay-snp-name-col", required=True)

    args = parser.parse_args()

    def parse_col(val):
        return None if val == "NA" else int(val)

    return {
        "assay_path": args.assay_path,
        "assay_design_path": args.assay_design_path,
        "species": args.species,
        "four_letter_species_code": args.four_letter_code,
        "assembly_version": args.assembly_version,
        "assay_design_snp_name_col": parse_col(args.assay_design_snp_name_col),
        "assay_design_snp_chrom_col": parse_col(args.assay_design_snp_chrom_col),
        "assay_design_snp_base_pos_col": parse_col(args.assay_design_snp_base_pos_col),
        "assay_design_snp_flank_col": None if args.assay_design_snp_flank_col == "NA" else args.assay_design_snp_flank_col,
        "assay_design_qual_col": parse_col(args.assay_design_qual_col),
        "assay_snp_name_col": parse_col(args.assay_snp_name_col)
    }

params = parse_args()

##############################################################################################################################################
# BUILDING STRINGS
fasta_filename = f"{params['four_letter_species_code']}.{params['assembly_version'].replace('v', '').replace('.', '_')}.fa.gz"
fasta_path = os.path.join(
    "/isg/treegenes/treegenes_store/FTP/Genomes",
    params['four_letter_species_code'],
    params['assembly_version'],
    "genome",
    fasta_filename
)

assay_name = os.path.splitext(os.path.basename(params['assay_path']))[0]
##############################################################################################################################################

def open_fasta_file(fasta_path):
    """
    Open FASTA file, handling both compressed (.gz) and uncompressed files.
    """
    if fasta_path.endswith('.gz'):
        return gzip.open(fasta_path, 'rt')
    else:
        return open(fasta_path, 'r')

def sequence_processing(df, flank_col):
    sequences = df[flank_col].astype(str)
    snp_matches = sequences.str.extract(r'(.*)(\[[A-Z]/[A-Z]\])(.*)', expand=True)
    
    offsets = snp_matches[0].str.len().fillna(0).astype(int)
    cleaned_sequences = sequences.str.replace(r'\[[A-Z]/[A-Z]\]', 'N', regex=True)
    
    return cleaned_sequences, offsets

# Function to map flanks to assembly
def remap_snp_probes(new_genome, snp_probes_csv, species, new_genome_version, assay_name):
    final_output = f"{assay_name}.FlanksMappedTo.{species}.{new_genome_version}.csv"
    tmpdir = Path(tempfile.mkdtemp(prefix="snp_remap_"))
    
    fasta_out = tmpdir / "temp_snps.fasta"
    snp_pos_out = tmpdir / "snp_positions.txt"
    bam_out = tmpdir / "aligned_snps_sorted.bam"
    bed_out = tmpdir / "aligned_snps.bed"
    remapped_out = tmpdir / "temp_remapped.fa"
    all_mappings_out = tmpdir / "all_mappings.csv"
    
    # Index genome if needed
    if not Path(f"{new_genome}.fai").exists():
        subprocess.run(["samtools", "faidx", new_genome], check=True)
    if not Path(f"{new_genome}.bwt").exists():
        subprocess.run(["bwa", "index", new_genome], check=True)

    # Read probe data
    df = pd.read_csv(snp_probes_csv)
    
    # Get column names for proper indexing
    snp_name_col = df.columns[params['assay_design_snp_name_col']]
    flank_col = df.columns[params['assay_design_snp_flank_col']]
    
    cleaned_sequences, offsets = sequence_processing(df, flank_col)

    names = df[snp_name_col].astype(str)
    fasta_headers = ">" + names

    fasta_content = []
    for header, seq in zip(fasta_headers, cleaned_sequences):
        fasta_content.extend([header, seq])

    positions_df = pd.DataFrame({
        'name': names,
        'offset': offsets
    })

    with open(fasta_out, "w") as f:
        f.write("\n".join(fasta_content))
    
    positions_df.to_csv(snp_pos_out, header=False, index=False)

    # Align and convert to BAM
    bwa_cmd = ["bwa", "mem", "-t", "1", new_genome, str(fasta_out)]
    view_cmd = ["samtools", "view", "-@1", "-bS", "-"]
    sort_cmd = ["samtools", "sort", "-@1", "-o", str(bam_out)]

    with subprocess.Popen(bwa_cmd, stdout=subprocess.PIPE) as p1:
        with subprocess.Popen(view_cmd, stdin=p1.stdout, stdout=subprocess.PIPE) as p2:
            subprocess.run(sort_cmd, stdin=p2.stdout, check=True)
    
    subprocess.run(["samtools", "index", str(bam_out)], check=True)

    # Convert to BED
    with open(bed_out, "w") as out:
        subprocess.run(["bedtools", "bamtobed", "-i", str(bam_out)], stdout=out, check=True)

    # Read BED and extract sequences from new genome
    bed_df = pd.read_csv(bed_out, sep="\t", header=None,
                         names=["chr", "start", "end", "name", "score", "strand"])

    def extract_sequences_bulk(bed_df, new_genome):
        regions = bed_df.apply(lambda r: f"{r['chr']}:{r['start']+1}-{r['end']}", axis=1).tolist()
        names = bed_df["name"].tolist()
        region_to_name = dict(zip(regions, names))
        
        try:
            result = subprocess.run(
                ["samtools", "faidx", new_genome] + regions,
                stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True, check=True
            )
        except subprocess.CalledProcessError:
            return [(row["name"], "N", row["chr"], row["start"], row["end"],
                    row["strand"], row["score"]) for _, row in bed_df.iterrows()]
        
        seq_dict = {}
        for block in result.stdout.strip().split(">"):
            if not block:
                continue
            lines = block.strip().split("\n")
            header = lines[0]
            seq = "".join(lines[1:]).upper()
            seq_dict[header] = seq
        
        # Final output: name, seq, plus other BED fields
        results = []
        for _, row in bed_df.iterrows():
            region = f"{row['chr']}:{row['start']+1}-{row['end']}"
            name = row["name"]
            seq = seq_dict.get(region, "N")
            results.append((name, seq, row["chr"], row["start"], row["end"],
                            row["strand"], row["score"]))
        
        return results


    extracted_seqs = extract_sequences_bulk(bed_df, new_genome)
    
    # Load SNP positions
    pos_df = pd.read_csv(snp_pos_out, header=None, names=["name", "snp_offset"])
    pos_dict = dict(pos_df.values)

    # Prepare mappings
    mappings = []
    snp_map_count = {}

    for name, seq, chr_, start, end, strand, score in extracted_seqs:
        offset = pos_dict.get(name, 0)
        start_1based = start + 1
        end_1based = end

        if offset > 0:
            pos = end_1based - offset + 1 if strand == "-" else start_1based + offset
        else:
            pos = (start_1based + end_1based) // 2

        if strand == "-":
            # Reverse complement
            base_map = str.maketrans("ACGTURYSWKMBDHVN", "TGCAAYRSWMKVHDBN")
            seq = seq[::-1].translate(base_map)

        snp_map_count[name] = snp_map_count.get(name, 0) + 1
        multi = "yes" if snp_map_count[name] > 1 else "no"
        mappings.append([name, chr_, start_1based, end_1based, strand, score, seq, multi, pos])

    map_df = pd.DataFrame(mappings, columns=["SNP_name", "chr", "start", "end", "strand",
                                              "quality", "new_sequence", "multimapped", "pos"])
    map_df.to_csv(all_mappings_out, index=False)

    # Filter for best per SNP
    best_snp = (map_df.sort_values(["SNP_name", "quality"], ascending=[True, False])
                     .drop_duplicates("SNP_name", keep='first'))

    # Filter for best at locus
    map_df["locus"] = map_df["chr"].astype(str) + ":" + map_df["start"].astype(str)
    best_locus = (map_df.sort_values(["locus", "quality"], ascending=[True, False])
                        .drop_duplicates("locus", keep='first'))

    # Inner merge on both criteria
    final_df = pd.merge(best_snp, best_locus, how="inner",
                        on=["SNP_name", "chr", "start", "end", "strand",
                            "quality", "new_sequence", "multimapped", "pos"])

    final_df.drop(columns=["locus"], errors="ignore").to_csv(final_output, index=False)

    # Cleanup
    shutil.rmtree(tmpdir)
    
    return final_output

# Batch FASTA queries
def get_reference_alleles_batch(fasta_path, positions):
    ref_alleles = {}
    chrom_to_positions = {}
    for chrom, pos in positions:
        chrom_to_positions.setdefault(chrom, set()).add(pos)

    for chrom, pos_set in chrom_to_positions.items():
        regions = [f"{chrom}:{pos}-{pos}" for pos in sorted(pos_set)]
        try:
            result = subprocess.run(
                ["samtools", "faidx", fasta_path, *regions],
                stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True, check=True
            )
        except subprocess.CalledProcessError:
            for pos in pos_set:
                ref_alleles[(chrom, pos)] = 'N'
            continue

        blocks = result.stdout.strip().split('>')
        for block in blocks:
            if not block.strip():
                continue
            lines = block.strip().split('\n')
            header = lines[0]
            seq = ''.join(lines[1:]).upper()
            match = re.match(r'(\S+):(\d+)-(\d+)', header)
            if match:
                c, p1, _ = match.groups()
                ref_alleles[(c, int(p1))] = seq if seq in ['A', 'C', 'G', 'T'] else 'N'

    return ref_alleles

def find_alternate_alleles_from_genotypes(row, reference_allele):
    genotype_columns = row.iloc[9:]  # Skip VCF format columns
    
    # Flatten all genotypes and count alleles
    all_alleles = ''.join(genotype_columns.astype(str))
    
    # Count frequency of each allele (excluding reference)
    allele_counts = {}
    for allele in all_alleles:
        if allele in ['A', 'C', 'T', 'G'] and allele != reference_allele:
            allele_counts[allele] = allele_counts.get(allele, 0) + 1
    
    if not allele_counts:
        return '.'
    
    # Sort alleles by frequency (descending)
    sorted_alleles = sorted(allele_counts.items(), key=lambda x: x[1], reverse=True)
    return ','.join(allele for allele, _ in sorted_alleles)

def transform_row(row):
    ref = row['REF']
    alt = row['ALT'].split(',') if row['ALT'] != '.' else []
    result = []
    for g in row.index[9:]:
        val = row[g]
        if val in ['.', './.', '--', 'nan'] or not isinstance(val, str):
            result.append('./.')
            continue
        alleles = list(val)
        gt = []
        for a in alleles[:2]:
            if a == ref:
                gt.append('0')
            elif a in alt:
                gt.append(str(alt.index(a)+1))
            else:
                gt.append('.')
        if len(gt) == 1:
            gt.append(gt[0])
        result.append(f"{gt[0]}/{gt[1]}")
    return pd.Series(result, index=row.index[9:])

def md5(sequence):
    return hashlib.md5(sequence.encode()).hexdigest()

def create_vcf_from_assay_data(assay_path, assay_design_path, remapped_flanks_path, fasta_path, 
                               species, assembly_version, assay_name, 
                               assay_design_snp_name_col, assay_design_snp_chrom_col, 
                               assay_design_snp_base_pos_col, assay_design_qual_col,
                               assay_snp_name_col):
    
    assay_data = pd.read_csv(assay_path, dtype=str)
    assay_design = pd.read_csv(assay_design_path, dtype=str)
    
    # Get column names
    snp_name_col = assay_design.columns[assay_design_snp_name_col]
    
    # Create base VCF table structure
    sample_cols = [col for col in assay_data.columns if col != assay_data.columns[assay_snp_name_col]]
    trees = sample_cols
    
    # Initialize VCF table with proper column structure
    vcf_data = {
        'ID': assay_design[snp_name_col].values,
    }
    
    # Set chromosome and position
    if remapped_flanks_path and os.path.exists(remapped_flanks_path):
        remapped_data = pd.read_csv(remapped_flanks_path).set_index("SNP_name")
        assay_design = assay_design.set_index(snp_name_col)
        vcf_data['CHROM'] = remapped_data.reindex(assay_design.index)["chr"].fillna('.').values
        vcf_data['POS'] = remapped_data.reindex(assay_design.index)["pos"].fillna('.').values
    else:
        # Use original design data if available
        if assay_design_snp_chrom_col is not None:
            chrom_col = assay_design.columns[assay_design_snp_chrom_col]
            vcf_data['CHROM'] = assay_design[chrom_col].values
        else:
            vcf_data['CHROM'] = ['.'] * len(assay_design)
            
        if assay_design_snp_base_pos_col is not None:
            pos_col = assay_design.columns[assay_design_snp_base_pos_col]
            vcf_data['POS'] = assay_design[pos_col].values
        else:
            vcf_data['POS'] = ['.'] * len(assay_design)
    
    # Set quality scores
    if assay_design_qual_col is not None:
        qual_col = assay_design.columns[assay_design_qual_col]
        vcf_data['QUAL'] = assay_design[qual_col].values
    else:
        vcf_data['QUAL'] = ['.'] * len(assay_design)
    
    # Add remaining standard VCF columns
    vcf_data['FILTER'] = ['PASS'] * len(assay_design)
    vcf_data['INFO'] = ['.'] * len(assay_design)
    vcf_data['FORMAT'] = ['GT'] * len(assay_design)
    
    # Create initial VCF table
    VCF_table = pd.DataFrame(vcf_data)
    
    # Add genotype data
    assay_snp_col = assay_data.columns[assay_snp_name_col]
    
    # Create a mapping from SNP name to genotype data
    assay_data_indexed = assay_data.set_index(assay_snp_col)
    
    # Add all genotype columns
    genotype_data = {}
    for sample in trees:
        if sample in assay_data_indexed.columns:
            genotype_data[sample] = VCF_table['ID'].map(assay_data_indexed[sample]).fillna('./.')
        else:
            genotype_data[sample] = ['./.' for _ in range(len(VCF_table))]
    
    # Concatenate all genotype columns
    genotype_df = pd.DataFrame(genotype_data)
    VCF_table = pd.concat([VCF_table, genotype_df], axis=1)
    
    VCF_table.set_index('ID', inplace=True)
    
    # Extract reference alleles from FASTA
    positions = [(str(row['CHROM']), int(row['POS']) if pd.notna(row['POS']) else 1) 
                for _, row in VCF_table.iterrows()]
    
    ref_alleles_dict = get_reference_alleles_batch(fasta_path, positions)
    
    # Map results back to VCF table
    ref_alleles = []
    for _, row in VCF_table.iterrows():
        chrom = str(row['CHROM'])
        pos = int(row['POS']) if pd.notna(row['POS']) else 1
        ref_alleles.append(ref_alleles_dict.get((chrom, pos), 'N'))
    
    # Find alternate alleles from genotype data
    alt_alleles = []
    for i, row in VCF_table.iterrows():
        alt_allele = find_alternate_alleles_from_genotypes(row, ref_alleles[len(alt_alleles)])
        alt_alleles.append(alt_allele)
    
    # Add REF and ALT columns
    VCF_table['REF'] = ref_alleles
    VCF_table['ALT'] = alt_alleles
    
    # Reset index to make ID accessible as a column again
    VCF_table.reset_index(inplace=True)
    
    standard_cols = ['CHROM', 'POS', 'ID', 'REF', 'ALT', 'QUAL', 'FILTER', 'INFO', 'FORMAT']
    sample_cols = [col for col in VCF_table.columns if col not in standard_cols]
    
    # Reorder the dataframe columns
    VCF_table = VCF_table[standard_cols + sample_cols]
    
    # Transform genotypes to VCF format
    VCF_table.iloc[:, 9:] = VCF_table.apply(transform_row, axis=1)
    
    # Remove rows with no genotype calls
    sample_columns = VCF_table.columns[9:]  # All columns after FORMAT
    VCF_table = VCF_table.loc[~(VCF_table[sample_columns].eq("./.").all(axis=1))]
    
    # Generate VCF header
    contig_IDs = [contig for contig in VCF_table['CHROM'].unique() if not pd.isna(contig)]
    
    # Calculate assembly length and get individual contig lengths and MD5 checksums
    assembly_length = 0
    contig_lengths = {}
    contig_md5s = {}
    fasta_ids = set()
    
    if not os.path.exists(fasta_path):
        raise FileNotFoundError(f"FASTA file not found at {fasta_path}")
    
    with open_fasta_file(fasta_path) as fasta_handle:
        for record in SeqIO.parse(fasta_handle, "fasta"):
            fasta_ids.add(record.id)
            contig_lengths[record.id] = len(record.seq)
            contig_md5s[record.id] = md5(str(record.seq))
            assembly_length += len(record.seq)
    
    # Generate VCF header
    vcf_header = [
        '##fileformat=VCFv4.3',
        f'##fileDate={datetime.now().strftime("%Y%m%d")}',
        '##source=TreeGenesVCFCreationProgram',
    ]
    
    # Add separate contig lines for each contig ID
    for contig_id in contig_IDs:
        contig_id_str = str(contig_id)
        matching_fasta_id = next((fasta_id for fasta_id in fasta_ids if contig_id_str in fasta_id), None)
        
        if matching_fasta_id:
            vcf_header.append(f'##contig=<ID={contig_id},length={contig_lengths[matching_fasta_id]},assembly={assembly_version},md5={contig_md5s[matching_fasta_id]},species="{species}">')
        else:
            vcf_header.append(f'##contig=<ID={contig_id},length=unknown,assembly={assembly_version},md5=unknown,species="{species}">')
    
    vcf_header.extend(['##FORMAT=<ID=GT,Number=1,Type=String,Description="Genotype">'])
    
    # Generate final VCF content
    header_line = "\t".join(["#CHROM", "POS", "ID", "REF", "ALT", "QUAL", "FILTER", "INFO", "FORMAT"] + trees)
    
    output_file_name = f"{assay_name}.{assembly_version}.vcf"
    output_path = os.path.join(".", output_file_name)
    
    # Write VCF file in chunks
    with open(output_path, "w") as output_file:
        # Write header
        output_file.write("\n".join(vcf_header) + "\n")
        output_file.write(header_line + "\n")
        
        # Write data in chunks
        chunk_size = 10000
        for i in range(0, len(VCF_table), chunk_size):
            chunk = VCF_table.iloc[i:i+chunk_size]
            chunk.to_csv(output_file, sep="\t", header=False, index=False, line_terminator="\n")
    
    return output_path

# Main execution
if __name__ == "__main__":
    # Determine if we need to remap flanking sequences
    needs_remapping = (params['assay_design_snp_flank_col'] != "NA" and (params['assay_design_snp_chrom_col'] == "NA" or params['assay_design_snp_base_pos_col'] == "NA"))
    
    remapped_flanks_path = None
    
    if needs_remapping:
        remapped_flanks_path = remap_snp_probes(
            new_genome=fasta_path,
            snp_probes_csv=params['assay_design_path'],
            species=params['species'],
            new_genome_version=params['assembly_version'],
            assay_name=assay_name
        )
    
    # Create VCF file from CSV data
    vcf_output_path = create_vcf_from_assay_data(
        assay_path=params['assay_path'],
        assay_design_path=params['assay_design_path'],
        remapped_flanks_path=remapped_flanks_path,
        fasta_path=fasta_path,
        species=params['species'],
        assembly_version=params['assembly_version'],
        assay_name=assay_name,
        assay_design_snp_name_col=params['assay_design_snp_name_col'],
        assay_design_snp_chrom_col=params['assay_design_snp_chrom_col'],
        assay_design_snp_base_pos_col=params['assay_design_snp_base_pos_col'],
        assay_design_qual_col=params['assay_design_qual_col'],
        assay_snp_name_col=params['assay_snp_name_col']
    )

