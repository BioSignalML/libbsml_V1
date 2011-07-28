/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _BSML_NAMES_H
#define _BSML_NAMES_H

#define BSML_NS            "http://www.biosignalml.org/ontologies/2011/04/biosignalml#"

// owl:Classes:
#define BSML_Annotation    (BSML_NS "Annotation")
#define BSML_BP_Filter     (BSML_NS "BP_Filter")
#define BSML_Dataset       (BSML_NS "Dataset")
#define BSML_Device        (BSML_NS "Device")
#define BSML_Filter        (BSML_NS "Filter")
#define BSML_Format        (BSML_NS "Format")
#define BSML_HP_Filter     (BSML_NS "HP_Filter")
#define BSML_LP_Filter     (BSML_NS "LP_Filter")
#define BSML_Notch_Filter  (BSML_NS "Notch_Filter")
#define BSML_Recording     (BSML_NS "Recording")
#define BSML_Repository    (BSML_NS "Repository")
#define BSML_Resolution    (BSML_NS "Resolution")
#define BSML_Signal        (BSML_NS "Signal")
#define BSML_Simulation    (BSML_NS "Simulation")
#define BSML_Source        (BSML_NS "Source")
#define BSML_Transducer    (BSML_NS "Transducer")
#define BSML_Type          (BSML_NS "Type")
#define BSML_UnitOfMeasure (BSML_NS "UnitOfMeasure")

// owl:ObjectProperties:
#define BSML_annotation    (BSML_NS "annotation")
#define BSML_filter        (BSML_NS "filter")
#define BSML_recording     (BSML_NS "recording")
#define BSML_repository    (BSML_NS "repository")
#define BSML_resolution    (BSML_NS "resolution")
#define BSML_signal        (BSML_NS "signal")
#define BSML_transducer    (BSML_NS "transducer")
#define BSML_time          (BSML_NS "time")
#define BSML_type          (BSML_NS "type")
#define BSML_units         (BSML_NS "units")

// owl:DataProperties:
#define BSML_sampleRate    (BSML_NS "sampleRate")
#define BSML_minFrequency  (BSML_NS "minFrequency")
#define BSML_maxFrequency  (BSML_NS "maxFrequency")
#define BSML_minValue      (BSML_NS "minValue")
#define BSML_maxValue      (BSML_NS "maxValue")
#define BSML_scale         (BSML_NS "scale")
#define BSML_offset        (BSML_NS "offset")
#define BSML_signalcount   (BSML_NS "signalcount")

// owl:Things:
  // Biosignal file formats
#define BSML_BioSignalML   (BSML_NS "BioSignalML")
#define BSML_SignalStream  (BSML_NS "SignalStream")
#define BSML_RAW           (BSML_NS "RAW")
#define BSML_EDF           (BSML_NS "EDF")
#define BSML_EDFplus       (BSML_NS "EDFplus")
#define BSML_FieldML       (BSML_NS "FieldML")
#define BSML_MFER          (BSML_NS "MFER")
#define BSML_SCP_ECG       (BSML_NS "SCP_ECG")
#define BSML_WFDB          (BSML_NS "WFDB")
#define BSML_SDF           (BSML_NS "SDF")
#define BSML_HDF5          (BSML_NS "HDF5")
#define BSML_CLOCK         (BSML_NS "CLOCK")

  // Types of biosignals
#define BSML_BP            (BSML_NS "BP")
#define BSML_ECG           (BSML_NS "ECG")
#define BSML_EEG           (BSML_NS "EEG")

#endif
