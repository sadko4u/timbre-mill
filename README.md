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

Note that input files may have different sample rates. The minimum sample rate
is considered as a cut-off frequency. Additionally, the transition zone (which is
0.5 octaves by default) is applied to cut-off frequency to reduce the effect of low-pass
cut-off filter applied to the signal at the discretization stage which may result in
significant amplification of frequencies near to the nyquist frequency. That's why the
frequencies above the computed cut-off frequency do not take part in frequency correction
and are not affected by the inverse impulse response.

There is additional 'mastering' mode provided that allows to swap roles between master and
child files so the timbral correction becomes applied to child files respective to the
computed spectral difference of master to the child.

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
	"transition_zone" : 0.5,
	"produce": [ "ir", "audio", "frm", "frc" ],
	"dry" : -12,
	"wet" : 0,
	"mastering" : false,
	"normalize": "above",
	"norm_gain": -6,
	"latency_compensation": false,
	"match_length": false,
	
	"ir": {
		"head_cut": 45,
		"tail_cut": 5,
		"fade_in": 2,
		"fade_out": 50,
		"file": "${group}/${master_name}/${file_name} - IR.wav",
		"raw": "${group}/${master_name}/${file_name} - Raw IR.wav",
		"fr_master": "${group}/${master_name}/${file_name} - FR Master.wav",
		"fr_child": "${group}/${master_name}/${file_name} - FR Child.wav"
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
  * **dry** - the loudness of dry (unprocessed) signal in dB in the output audio file, by default -1000 dB;
  * **dst_path** - destination path to store output files (empty by default);
  * **fft_rank** - the FFT rank (from 8 to 16) to use for the analysis, 12 by default (4096 samples);
  * **file** - the format of the processed audio file name, by default "${master_name}/${file_name} - processed.wav";
  * **groups** - the key-value map between group name and it's description:
    * **master** - the name of the master file (absolute path name or relative to the **src_path** directory);
    * **files** - the list of child files (absolute path name or relative to the **src_path** directory);
  * **ir** - the parameters of output IR file:
    * **head_cut** - the amount of data (in percent) to cut from the IR file at the beginning;
    * **tail_cut** - the amount of data (in percent) to cut from the IR file at the end;
    * **fade_in** - the amount of linear fade-in (in percent) to add at the beginning of the IR file;
    * **fade_out** - the amount of linear fade-in (in percent) to add at the end of the IR file;
    * **file** - the name of the stripped impulse response file, by default "${master_name}/${file_name} - IR.wav";
    * **fr_master** - the name of the impulse response file with the frequency response that matches the master file,
      by default "${master_name}/${file_name} - FR Master.wav";
    * **fr_child** - the name of the impulse response file with the frequency response that matches the child file,
      by default "${master_name}/${file_name} - FR Child.wav";
    * **raw** - the name of the raw impulse response file, by default "${master_name}/${file_name} - Raw IR.wav";
  * **latency_compensation** - remove extra samples that introduce latency from the beginning of the processed file;
  * **masetering** - enables the tool working in reverse mode (applying timbral correction from master to child files);
  * **match_length** - remove extra samples from the output file to match the length of the source file.
  * **norm_gain** - the peak normalization gain (in decibels) at output;
  * **normalize** - the output file normaliztion:
    * **none** - do not use normalization (default);
    * **above** - normalize the file if the maximum signal peak is above the **norm_gain** level;
    * **below** - normalize the file if the maximum signal peak is below the **norm_gain** level;
    * **always** - always normalize output files to match the maximum signal peak to **norm_gain** level;
  * **produce** - the array of strings that indicates the list of files to produce, ```[ "all" ]``` by default:
    * **all** - produce all types of files: IR, raw IR, processed audio;
    * **audio** - produce processed audio file;
    * **frc** - produce IR file that matches frequency response of the child file;
    * **frm** - produce IR file that matches frequency response of the master file;
    * **ir** - produce IR file;
    * **raw** - produce raw IR file;
  * **srate** - the sample rate for output files (IR, stripped IR and the processed master files), default 48000;
  * **src_path** - source path to take files from (empty by default);
  * **transition_zone** - the value of the frequency transition zone (in octaves);
  * **wet** - the loudness of wet (processed) signal in dB in the output audio file, by default 0 dB.

For the **dry**/**wet** balance values below -150 dB are considered as negative infinite gain.
The values above 150 dB are constrained to +150 dB.

Each name of the output file can be parametrized with the following predefined values:
  * **file** - the name of the child file without any parent directory, for example "trp plunger.wav";
  * **file_ext** - the extension of the child file for example "wav";
  * **file_name** - the name of the child  file without any parent directory and additionally removed file extension, for example "trp plunger";
  * **group** - the name of the group related to the file;
  * **master** - the name of the master file without any parent directory, for example "trp unmuted.wav";
  * **master_ext** - the extension of the master file for example "wav";
  * **master_name** - the name of the master file without any parent directory and additionally removed file extension, for example "trp unmuted";
  * **srate** - the sample rate of output file.
    
The tool allows to override some batch parameters by specifying them as command-line arguments. The full list can be obtained by issuing ```timbre-mill --help``` command and is the following:

```
  -c, --config                   Configuration file name (required if no -mf option is set)
  -cf, --child                   The name of the child file (multiple options allowed)
  -d, --dst-path                 Destination path to store audio files
  -dg, --dry                     The amount (in dB) of unprocessed signal in output file
  -f, --file                     Format of the output file name
  -fr, --fft-rank                The FFT rank (resolution) used for profiling
  -frc, --fr-child               The name of the frequency response file for the child file
  -frm, --fr-master              The name of the frequency response file for the master file
  -g, --group                    The group name for -cf (--child) option, "default" if not set
  -h, --help                     Output this help message
  -ir, --ir-file                 Format of the processed impulse response file name
  -iw, --ir-raw                  Format of the raw impulse response file name
  -ifi, --ir-fade-in             The amount (in %) of fade-in for the IR file
  -ifo, --ir-fade-out            The amount (in %) of fade-out for the IR file
  -ihc, --ir-head-cut            The amount (in %) of head cut for the IR file
  -itc, --ir-tail-cut            The amount (in %) of tail cut for the IR file
  -lc, --latency-compensation    The amount (in %) of tail cut for the IR file
  -m, --mastering                Work as auto-mastering tool instead of timbral correction
  -mf, --master                  The name of the master file
  -ml, --match-length            Match the length of the output file to the input file
  -n, --normalize                Set normalization mode
  -ng, --norm-gain               Set normalization peak gain (in dB)
  -p, --produce                  Comma-separated list of produced output files (ir,frm,frc,raw,audio,all)
  -s, --src-path                 Source path to take files from
  -sr, --srate                   Sample rate of output files
  -tz, --transition-zone         The value of the frequency transition zone (in octaves)
  -wg, --wet                     The amount (in dB) of processed signal in output file

```

If the option ```-mf``` is specified, the default value of ```-p``` option is reset to ```audio```. Additionally:
* Specifying the ```-ir``` option automatically adds the ```ir``` item to the ```-p``` option.
* Specifying the ```-frc``` and ```-frm``` options automatically adds the ```frm``` and ```frc```
  items respectively to the ```-p``` option.
* The same behaviour is also true for ```-iw``` option which automatically adds the ```raw``` item to the ```-p``` option.
* Explicitly specified ```-p``` option won't be overridden by the ```-mf```, ```-ir``` and ```-iw``` options.

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


