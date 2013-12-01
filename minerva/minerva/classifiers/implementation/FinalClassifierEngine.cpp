/*	\file   FinalClassifierEngine.cpp
	\date   Sunday August 11, 2013
	\author Gregory Diamos <solusstultus@gmail.com>
	\brief  The source file for the FinalClassifierEngine class.
*/

// Minerva Includes
#include <minerva/classifiers/interface/FinalClassifierEngine.h>
#include <minerva/classifiers/interface/Classifier.h>

#include <minerva/util/interface/debug.h>

namespace minerva
{

namespace classifiers
{

FinalClassifierEngine::FinalClassifierEngine()
{

}	

float FinalClassifierEngine::getAccuracy() const
{
	size_t matches = 0;
	size_t total   = 0;

	for(auto& labelStatistic : _statistics.labelStatistics)
	{
		total += labelStatistic.second.truePositives;
		total += labelStatistic.second.trueNegatives;
		total += labelStatistic.second.falsePositives;
		total += labelStatistic.second.falseNegatives;

		matches += labelStatistic.second.truePositives;
		matches += labelStatistic.second.trueNegatives;
	}
	
	return (matches + 0.0) / total;
}

void FinalClassifierEngine::reportStatistics(std::ostream& stream) const
{
	stream << _statistics.toString();
}

void FinalClassifierEngine::runOnImageBatch(const ImageVector& images)
{
	Classifier classifier(*_model);
	
	auto labels = classifier.classify(images);
	
	assert(labels.size() == images.size());

	auto image = images.begin();
	for(auto label = labels.begin(); label != labels.end() &&
		image != images.end(); ++label, ++image)
	{
		util::log("FinalClassifierEngine") << " Classified '" << image->path()
			<< "' as '" << *label << "'\n";
		
		_updateStatistics(*label, *image);
	}

	// TODO interpret labels, update statistics
}

size_t FinalClassifierEngine::getInputFeatureCount() const
{
	Classifier classifier(*_model);
	
	return classifier.getInputFeatureCount();
}

std::string FinalClassifierEngine::Statistics::toString() const
{
	std::stringstream stream;

	stream << "Statistics for 'Classifier' Neural Network:\n";

	for(auto& labelStatistic : labelStatistics)
	{
		stream << " for neuron with label '" << labelStatistic.second.label << "':\n";
		
		stream << "  true  positives: " << labelStatistic.second.truePositives  << "\n";
		stream << "  true  negatives: " << labelStatistic.second.trueNegatives  << "\n";
		stream << "  false positives: " << labelStatistic.second.falsePositives << "\n";
		stream << "  false negatives: " << labelStatistic.second.falseNegatives << "\n";
	}

	return stream.str();
}

void FinalClassifierEngine::_updateStatistics(const std::string& label,
	const Image& image)
{
	// Add trackers for all possible labels if they exist
	auto& classifier = _model->getNeuralNetwork("Classifier");
	
	for(unsigned i = 0; i < classifier.getOutputCount(); ++i)
	{
		auto outputLabel = classifier.getLabelForOutputNeuron(i);

		if(outputLabel.empty()) continue;

		if(_statistics.labelStatistics.count(outputLabel) != 0)
		{
			continue;
		}
		
		_statistics.labelStatistics.insert(std::make_pair(outputLabel,
			LabelStatistic(outputLabel, 0, 0, 0, 0)));
	}

	// Update the statistics for each label
	for(auto& labelStatistic : _statistics.labelStatistics)
	{
		if(label == labelStatistic.second.label)
		{
			// match 
			if(label == image.label())
			{
				labelStatistic.second.truePositives += 1; 
			}
			else
			{
				labelStatistic.second.falsePositives += 1;
			}
		}
		else if (image.label() == labelStatistic.second.label)
		{
			labelStatistic.second.falseNegatives += 1;
		}
		else
		{
			labelStatistic.second.trueNegatives += 1;
		}
	}
}

FinalClassifierEngine::LabelStatistic::LabelStatistic(const std::string& l,
	size_t tp, size_t tn, size_t fp, size_t fn)
: label(l), truePositives(tp), trueNegatives(tn), falsePositives(fp),
	falseNegatives(fn)
{

}

}

}


