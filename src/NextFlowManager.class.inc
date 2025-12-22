<?php

/**
 * @file
 * Class 'SlurmSchedular' which manages submitting jobs to HPC.
 */

/**
 * Manages HPC Jobs.
 *
 * @package tpps
 * @author Gabriel Barrett <gbarrett9086@gmail.com>
 */
class NextFlowManager {

    /**
     * Username for ssh connection.
     */
    const USER = 'tg-nginx';

    /**
     * HostName for ssh connection.
     */
    const HOST = 'login.hpc.cam.uchc.edu';

    /**
     * NextFlow Workflow Name as it relates to .nextflow/assets directory
     *
     * @var string
     */
    protected $pipeline_name;

    /**
     * Path to output directory from nextflow run
     *
     * @var string
     */
    protected $out_dir;

    /**
     * Path to scratch directory during nextflow run
     *
     * @var string
     */
     protected $scratch_dir;

    /**
     * Two dimensional array of nextflow arguments (argument:value)
     *
     * @var list
     */
    protected $nxf_arguments = [];

    /**
     * Job ID acquired after running submitJob()
     *
     * @var string
     */
    private $jobId;

    /**
     * Class constructor.
     *
     * Throws Exception: Failed to connect
     */
    public function __construct($pipeline_name, $out_dir, $scratch_dir, $nodesToExc)
    {
        $this->pipeline_name = $pipeline_name;
        $this->out_dir = $this->createDirIfNotExist($out_dir);
	    $this->scratch_dir = $scratch_dir;
        $this->addNextflowArgument('r', 'main', true);
        if ($nodesToExc !== NULL) {
            $this->addNextflowArgument('c', $this->writeConfig($nodesToExc), true);
        } else {
            $this->addNextflowArgument('profile', 'xanadu', true);
        }
        $this->addNextflowArgument('resume', '', false);
    }

    /** 
     * Should be called within the constructor
     * 
     * @param nxf running and output dir
     * @return output directory path
     */
    private function createDirIfNotExist($out_dir)
    {
        if (!is_dir($out_dir)) {
            if (!mkdir($out_dir, 0755, true) && !is_dir($out_dir)) {
                throw new RuntimeException("Failed to create directory: $out_dir");
            }
        }
        return $out_dir;
    }

    private function getShebang()
    {
        return '#!/bin/bash' . "\n";
    }

    /**
     * @return String slurm header
     */
    private function getSlurmHeader()
    {
        return
            "#SBATCH --job-name=${pipeline_name}" . "\n" .
            '#SBATCH -N 1' . "\n" .
            '#SBATCH -n 1' . "\n" .
            '#SBATCH -c 1' . "\n" .
            '#SBATCH --partition=general' . "\n" .
            '#SBATCH --qos=general' . "\n" .
            '#SBATCH --mail-type=END' . "\n" .
            '#SBATCH --mem=10G' . "\n" .
            '#SBATCH --mail-user=tg-nginx@cam.uchc.edu' . "\n" .
            '#SBATCH -o ' . $this->out_dir . '/assay_design_%j.out' . "\n" .
            '#SBATCH -e ' . $this->out_dir. '/assay_design_%j.err' . "\n" .
            'source /etc/profile.d/modules.sh' . "\n" .
            'module load nextflow' . "\n" .
            'mkdir -p "' . $this->scratch_dir . '/tg-nginx/new_study_pipeline_$SLURM_JOB_ID"' . "\n" .
            'export TMPDIR=' . $this->scratch_dir . '/tg-nginx/new_study_pipeline_$SLURM_JOB_ID' . "\n" .
            'export NXF_TEMP=' . $this->scratch_dir. '/tg-nginx/new_study_pipeline_$SLURM_JOB_ID' . "\n" .
            'export NXF_WORK=' . $this->scratch_dir . '/tg-nginx/new_study_pipeline_$SLURM_JOB_ID' . "\n" .
            'export NXF_OPTS=\'-Xms5G -Xmx9G\'' . "\n" .
            'export TZ=\'America/New_York\'' . "\n" .
            'cd ' . $this->out_dir . "\n";
    }

    /**
     * Adds to Nextflow arguments list for building the invocation string.
     *  Checks if providing path to file not mounted on mantis and copies it to out_dir
     *
     * @param string $arg
     *   The argument name.
     * @param string $value
     *   The argument value.
     * @param bool $nextflow
     *   Whether it should use a single dash (e.g. `-r`) or a double dash (e.g. `--profile`).
     *
     * @return void
     */
    public function addNextflowArgument(string $arg, string $value, bool $nextflow=false)
    {
        if (strpos($value, '/var/www/Drupal') !== false) {
            $destination = $this->out_dir . '/' . basename($value);
            if (copy($value, $destination)) {
                $value = $destination;
            } else {
                throw new Exception ("Failed to copy file: " . $value . " to directory " . $this->out_dir);
            }
        }

        $this->nxf_arguments[] = [$arg, $value, $nextflow];
    }

    /**
     * Builds the Nextflow argument string from the internal 3D array.
     *
     * 3rd column indicates if the argument should use a single dash (-) or double dash (--).
     *
     * @return string
     *   A space-separated string of properly formatted Nextflow arguments.
     */
    public function buildNextflowArguments(): string {
        $parts = [];

        foreach ($this->nxf_arguments as $item) {
            list($arg, $value, $nextflow) = $item;
            if ($nextflow) {
                $parts[] = '-' . $arg . ' ' . escapeshellarg($value);
            } else {
                $parts[] = '--' . $arg . ' ' . escapeshellarg($value);
            }
        }

        return implode(' ', $parts);
    }

    /**
     * @return string
     * Returns CLI for running nextflow workflow
     */
    private function getNextflowCommand(): string
    {
        return "nextflow run " . $this->pipeline_name . " " . $this->buildNextflowArguments();
    }

    /**
     * @return string
     * Concats slurm header and nextflow command
     */
    private function getSlurmScript(): string
    {
        return $this->getShebang() . $this->getSlurmHeader() . "\n\n" . $this->getNextflowCommand() . "\n";
    }

    /**
     * @return string|null
     * Execute a shell command and return output as string.
     */
    private function runCommand(string $cmd)
    {
        $output = [];
        $returnCode = 0;
        exec($cmd, $output, $returnCode);

        if ($returnCode !== 0 || empty($output)) {
            return null;
        }
        return trim($output[0]);
    }

    /**
     * @return string status
     * Returns the status of the current jobId
     */
    public function getJobStatus()
    {
        // Build the SSH + sacct command in one sprintf string
        $cmd = sprintf(
            'ssh %s@%s "sacct --jobs=%s --format=State -X --parsable2 | tail -n1"',
            escapeshellarg(self::USER),   // SSH username
            escapeshellarg(self::HOST), // SSH host
            escapeshellarg($this->jobId)  // Job ID
        );
        return $this->runCommand($cmd);
    }

    /**
     * @return string
     * Waits for SLURM status to change to a final state.
     * @throws Exception if status returns null to many times.
     */
    public function hasCompleted($interval=5)
    {
        $finalStates = [
            'COMPLETED', 'FAILED', 'CANCELLED', 'TIMEOUT', 'PREEMPTED',
            'BOOT_FAIL', 'NODE_FAIL', 'OUT_OF_MEMORY',
        ];

        $retries = 0;
        $maxRetries = 5;
        while (true) {
            $status = $this->getJobStatus();

            if ($status === null && $retries < $maxRetries) {
                sleep($interval);
                $retries++;
            } elseif ($status === null) {
                throw new Exception("Reached maximum number of retries for getting job status.");
            }

            if (in_array($status, $finalStates, true)) {
                return $status;
            }

            sleep($interval);
        }
    }

    /**
     * @return boolean
     * Checks if final state was successful
     */
    public function isSuccessful($status): bool
    {
        if ($status != 'COMPLETED') {
            return false;
        }
        return true;
    }

    /**
     * Recursively removes files in a directory
     * @param string path to directory
     */
    public function nukeDir()
    {
    }

    /**
     * @param filePath file to write to
     * @param content contents to write into file
     * @return void
     * Writes string contents to filepath
     */
    public function writeFile(string $filePath, string $content)
    {
        $fp = fopen($filePath, "w") or die("I can't write file due to permissions.");
        fwrite($fp, $content);
        fclose($fp);
    }

    /**
     * Executes sbatch on cluster
     *
     * @throws Exception Failed to connect or Failed to get jobId
     * @return jobId the SLURM job ID for monitoring status 
     */
    public function submitJob(): string
    {
        $filePath = $this->out_dir . '/' . 'submitJob.sh';
        $this->writeFile($filePath, $this->getSlurmScript());

        $cmd = sprintf(
            "ssh %s@%s 'sbatch %s 2>&1'",
            escapeshellarg(self::USER),
            escapeshellarg(self::HOST),
            escapeshellarg($filePath)
        );

        $out = $this->runCommand($cmd);

        if (preg_match('/Submitted batch job (\d+)/', $out, $matches)) {
            $this->jobId = $matches[1];
            return $matches[1]; // Job ID
        } else {
            throw new Exception('Did not receive expected output after sbatch. Could not grab a jobId');
        }
    }

    /**
     * Finds the most recently modified Nextflow log file in a directory.
     *
     * @param string|null $dir Directory to search; defaults to $this->out_dir
     * @return string|null Full path to the newest .nextflow.log file, or null if none found
     */
    public function getLatestNextflowLog(string $dir = null)
    {
        $dir = $dir ?? $this->out_dir;

        if (!is_dir($dir)) {
            return null;
        }

        $files = glob(rtrim($dir, '/') . '/.nextflow.log*');
        if (empty($files)) {
            return null;
        }

        // Sort by modification time descending
        usort($files, function($a, $b) {
            return filemtime($b) <=> filemtime($a);
        });

        return $files[0];
    }

    /**
     * Extracts the first ERROR line and all lines after it from a log file.
     *
     * @param string|null $filePath Optional log file path; defaults to latest .nextflow.log in $this->out_dir
     * @return string|null Returns stacktrace string or null if none found
     */
    public function extractStacktrace($filePath = null)
    {
        // Use latest Nextflow log if no file path provided
        if ($filePath === null) {
            $filePath = $this->getLatestNextflowLog();
            if ($filePath === null) {
                return null; // no log files
            }
        }

        if (!is_readable($filePath)) {
            return null;
        }

        $lines = file($filePath, FILE_IGNORE_NEW_LINES);
        $stacktrace = [];
        $capture = false;

        foreach ($lines as $line) {
            // Start capturing at the first ERROR line
            if (!$capture && preg_match('/\berror\b/i', $line)) {
                $capture = true;
            }

            if ($capture) {
                $stacktrace[] = $line;
            }
        }

        if (empty($stacktrace)) {
            return null;
        }

        return implode("\n", $stacktrace);
    }

    /**
     * Writes nxf config file to exclude certain nodes to nxf run
     *  This is to resolve issues with certain nodes running nxf since this requires singularity etc.
     * @param nodesToExc nodes for masking
     * @return configFileName path to config
     */
    public function writeConfig($nodesToExc) {
        $configFileName = $this->out_dir . "/" . 'clusterOptions.config';
        $configFile = fopen($configFileName, "w") or die("Unable to open config file " . $configFileName);
    
        $clusterOptionsLine = "clusterOptions = { [ task.memory <= 245.GB ? '--qos=general' : '--qos=himem', '--exclude=" . $nodesToExc . "' ].join(' ').trim() }";
    
        $configContent = 
    "params {\n" .
    "    config_profile_description = 'The UConn HPC profile'\n" .
    "    config_profile_contact     = 'noah.reid@uconn.edu'\n" .
    "    config_profile_url         = 'https://bioinformatics.uconn.edu/'\n\n" .
    "    // max resources\n" .
    "    max_memory                 = 2.TB\n" .
    "    max_cpus                   = 64\n" .
    "    max_time                   = 21.d\n\n" .
    "    // Path to shared singularity images\n" .
    "    singularity_cache_dir      = '/isg/shared/databases/nfx_singularity_cache'\n" .
    "}\n\n" .
    "process {\n" .
    "    resourceLimits = [\n" .
    "        memory: 2.TB,\n" .
    "        cpus: 64,\n" .
    "        time: 21.d\n" .
    "    ]\n" .
    "    executor       = 'slurm'\n" .
    "    queue          = { task.memory <= 245.GB ? 'general' : (task.memory <= 512.GB ? 'himem' : 'himem2') }\n\n" .
    "    " . $clusterOptionsLine . "\n" .
    "}\n\n" .
    "executor {\n" .
    "    name            = 'slurm'\n" .
    "    submitRateLimit = '2 sec'\n" .
    "    queueSize       = 100\n" .
    "}\n\n" .
    "singularity {\n" .
    "    enabled       = true\n" .
    "    cacheDir      = params.singularity_cache_dir\n" .
    "    autoMounts    = true\n" .
    "    conda.enabled = false\n" .
    "}\n";
    
        fwrite($configFile, $configContent);
        fclose($configFile);
    
        return $configFileName;
    }    
}
?>
