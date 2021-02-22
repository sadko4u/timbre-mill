# timbre-mill

Timbre Mill is a tool for timbral correction of audio files.

The tool allows to specify multiple file groups containing two type of files:
  * master file;
  * child files.

For each file group, the tool performs spectral analysis of master file
and each child file. After that, it computes the raw inverse impulse responses
which allow to tune tonal characteristics of the master file to match
the tonal characteristics of each corresponding child file. These raw IR files
allow to process audio data as linear-phase finite impulse response filters (FIRs).

Since linear-phased filters are symmetric across the center of the impulse response,
the processed audio data will introduce latency of the half of the impulse response duration.
To reduce the latency effect, additional option is available to cut (or strip) the raw
impulse response and produce the stripped impulse response. This is done on the next step
after computing the raw impulse response.

At the last step, the stripped impulse reponse is applied to the master file and stored
as a separate file.

To automate and simplify workflow, the tool uses JSON configuration files as a batch file.

The example of batch file is the following:

```JSON
{
	"srate": 48000,
	"dst_path": "/home/user/out",
	"src_path": "/home/user/in",
	"file": "${group}/${master_name}/${file_name} - processed.wav",
	"fft_rank": 16,
	
	"ir": {
		"head_cut": 45,
		"tail_cut": 5,
		"fade_in": 2,
		"fade_out": 50,
		"file": "${group}/${master_name}/${file_name} - IR.wav",
		"raw": "${group}/${master_name}/${file_name} - Raw IR.wav"
	},

	"groups": {
		"trombone": {
			"master": "trombone/trb unmuted.wav",
			"files": [
				"trombone/trb harmon stem in.wav",
				"trombone/trb humes and berg cup.wav",
				"trombone/trb plunger.wav",
				"trombone/trb solotone.wav",
				"trombone/trb tom crown straight.wav"
			]
		},
		"trumpet": {
			"master": "trumpet/trp unmuted.wav",
			"files": [
				"trumpet/trp alessi vacchiano straight.wav",
				"trumpet/trp jo ral bubble stem out.wav",
				"trumpet/trp plunger.wav"
			]
		}
	}
}
```

Here's the full description of all possible parameters which can be omitted in the batch:
  * **srate** - the sample rate for output files (IR, stripped IR and the processed master files), default 48000;
  * **dst_path** - destination path to store output files (empty by default);
  * **src_path** - source path to take files from (empty by default);
  * **file** - the format of the processed audio file name, by default "${master_name}/${file_name} - processed.wav";
  * **fft_rank** - the FFT rank (from 8 to 16) to use for the analysis, 12 by default (4096 samples);
  * **ir** - the parameters of output IR file:
    * **head_cut** - the amount of data (in percent) to cut from the IR file at the beginning;
    * **tail_cut** - the amount of data (in percent) to cut from the IR file at the end;
    * **fade_in** - the amount of linear fade-in (in percent) to add at the beginning of the IR file;
    * **fade_out** - the amount of linear fade-in (in percent) to add at the end of the IR file;
    * **file** - the name of the stripped impulse response file, by default "${master_name}/${file_name} - IR.wav";
    * **raw** - the name of the raw impulse response file, by default "${master_name}/${file_name} - Raw IR.wav";
  * **groups** - the key-value map between group name and it's description:
    * **master** - the name of the master file (absolute path name or relative to the **src_path** directory);
    * **files** - the list of child files (absolute path name or relative to the **src_path** directory).

Each name of the output file can be parametrized with the following predefined values:
  * **srate** - the sample rate of output file;
  * **group** - the name of the group related to the file;
  * **master** - the name of the master file without any parent directory, for example "trp unmuted.wav";
  * **master_name** - the name of the master file without any parent directory and additionally removed file extension, for example "trp unmuted";
  * **master_ext** - the extension of the master file for example "wav";
  * **file** - the name of the child file without any parent directory, for example "trp plunger.wav";
  * **file_name** - the name of the child  file without any parent directory and additionally removed file extension, for example "trp plunger";
  * **file_ext** - the extension of the child file for example "wav";
    
The tool allows to override some batch parameters by specifying them as command-line arguments. The full list can be obtained by issuing ```timbre-mill --help``` command and is the following:

```
  -c, --config           Configuration file name (required)
  -d, --dst-path         Destination path to store audio files
  -f, --file             Format of the output file name
  -fr, --fft-rank        The FFT rank (resolution) used for profiling
  -h, --help             Output help message
  -ir, --ir-file         Format of the processed impulse response file name
  -iw, --ir-raw          Format of the raw impulse response file name
  -ifi, --ir-fade-in     The amount (in %) of fade-in for the IR file
  -ifo, --ir-fade-out    The amount (in %) of fade-out for the IR file
  -ihc, --ir-head-cut    The amount (in %) of head cut for the IR file
  -itc, --ir-tail-cut    The amount (in %) of tail cut for the IR file
  -s, --src-path         Source path to take files from
  -sr, --srate           Sample rate of output files
```

Requirements
======

The following packages need to be installed for building:

* gcc >= 4.9
* GNU make >= 4.0
* libsndfile
* libiconv

Building
======

To build the tool, perform the following commands:

```bash
make config # Configure the build
make fetch # Fetch dependencies from Git repository
make
sudo make install
```

To get more build options, run:

```bash
make help
```

To uninstall library, simply issue:

```bash
make uninstall
```

To clean all binary files, run:

```bash
make clean
```

To clean the whole project tree including configuration files, run:

```bash
make prune
```

To fetch all possible dependencies and make the source code tree portable between
different architectures and platforms, run:

```bash
make tree
```

To build source code archive with all possible dependencies, run:

```bash
make distsrc
```

