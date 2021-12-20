#include <random>
#include <inttypes.h>
#include <typeinfo>
#include "cifar.h"

#define RATIO_NEW_SAMPLES 0.5

cifar::CIFAR10_dataset<std::vector, std::vector<uint8_t>, uint8_t> CIFAR::tmp_dataset(cifar::read_dataset());
cifar::CIFAR10_dataset<std::vector, std::vector<double>, uint8_t> CIFAR::dataset;
cifar::CIFAR10_dataset<std::vector, std::vector<double>, uint8_t> CIFAR::subset;

void CIFAR::changeCurrentImage()
{
	// Get the container for the current mode.
	std::vector<std::vector<double>>& dataSource = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->dataset.training_images : this->dataset.test_images;

	this->currentIndex = (this->currentMode == Learn::LearningMode::TRAINING) ?
	this->rng.getUnsignedInt64(0, dataSource.size()-1) : (this->currentIndex + 1) % dataSource.size();

	// Load the image in the dataSource
	this->currentImage.setPointer(&dataSource.at(this->currentIndex));
	this->currentClass = (this->currentMode == Learn::LearningMode::TRAINING) ?
	this->dataset.training_labels.at(this->currentIndex) : this->dataset.test_labels.at(this->currentIndex);
}

CIFAR::CIFAR() : LearningEnvironment(2), currentImage(32, 32), currentClass(0), tp(0), tn(0), fp(0), fn(0), score(0)
{
	processDataset();
	// Fill shared tmp_dataset tmp_dataset(cifar::read_dataset<std::vector, std::vector, double, uint8_t>(CIFAR_DATA_LOCATION))
	/*if (this->dataset.training_labels.size() != 0) {
		std::cout << "Nbr of training images = " << dataset.training_images.size() << std::endl;
		std::cout << "Nbr of training labels = " << dataset.training_labels.size() << std::endl;
		std::cout << "Nbr of test images = " << dataset.test_images.size() << std::endl;
		std::cout << "Nbr of test labels = " << dataset.test_labels.size() << std::endl;
	}
	else {
		throw std::runtime_error("Initialization of CIFAR databased failed.");
	}*/
}

void CIFAR::doAction(uint64_t actionID)
{
	static int r = (dataset.training_images.size()-50*RATIO_IMB)/50*RATIO_IMB;
	// Call to default method to increment classificationTable
	if(this->currentClass == 0 && actionID == 0) //TP
	{
		//this->score += 1;  // basic TPG reward
		this->score += r;  // linear compensation
		this->tp += 1;       // GScore, MCC or
	}
	else if(this->currentClass == 1 && actionID == 1) //TN
	{
		this->score += 1;
		this->tn += 1;
	}
	else if (this->currentClass == 1 && actionID == 0) //FP
	{
		//this->score -= 1;   //basic TPG reward
		this->score -= r;   //linear compensation
		this->fp += 1;
	}
	else if (this->currentClass == 0 && actionID == 1) //FN
	{
		this->score -= 1;
		this->fn += 1;
	}

	LearningEnvironment::doAction(actionID);
	this->changeCurrentImage();
}

void CIFAR::reset(size_t seed, Learn::LearningMode mode)
{
	// Reset the classificationTable
	//ClassificationLearningEnvironment::reset(seed);
	this->currentMode = mode;
	this->rng.setSeed(seed);
	this->score = 0;
	this->tp = 0;
	this->tn = 0;
	this->fp = 0;
	this->fn = 0;
	
	// Reset at -1 so that in TESTING mode, first value tested is 0.
	this->currentIndex = -1;
	this->changeCurrentImage();
}

std::vector<std::reference_wrapper<const Data::DataHandler>> CIFAR::getDataSources()
{
	std::vector<std::reference_wrapper<const Data::DataHandler>> res = { currentImage };

	return res;
}

bool CIFAR::isCopyable() const
{
	return true;
}

Learn::LearningEnvironment* CIFAR::clone() const
{
	return new CIFAR(*this);
}

double CIFAR::getScore() const
{

	//MCC
	#if FITNESS==2
	int num = tp*tn-fp*fn;
	double densquare = (tp+fp)*(tp+fn)*(tn+fp)*(tn+fn);
	if(densquare <= 0)
	{
		return -100;
	}
	double res = (double)num / sqrt(densquare);
	return res;
	#endif

	//kappa
	#if FITNESS==1
    double sum = tp + tn + fp + fn;
    
    double P_agree = (double)(tp + tn)/(double)sum;
    
    double P_tp = (double)(tp+fp)/(double)sum * (double)(tp + fn)/(double)sum;
    double P_tn = (double)(tn+fp)/(double)sum * (double)(tn + fn)/(double)sum;
    
    double P_rand = P_tp + P_tn; 
    
    double kappa = (P_agree - P_rand)/(1-P_rand);
    return kappa;
    #endif

	//custom cost based 1
	/*int num = tp*tn-fp*fn;
	double densquare = (tp+fp)*(tp+fp)*(tn+fp)*(tn+fn);
	if(densquare <= 0)
	{
		return -100;
	}
	double res = (double)num / sqrt(densquare);
	return res;*/

	//GSCORE
	#if FITNESS==3
	double sen = (double)tp/(double)(tp+fn);
	double spe = (double)tn/(double)(tn+fp);
	if(sen*spe!=0)
	{
		return (sqrt(sen * spe));
	}
	return -10;
	#endif

	//linear
    #if FITNESS==0
	return (double)this->score/100.0; // Basic TGP reward and linear compensation
	#endif

	//F1
	#if FITNESS==4
	//f1 score
	double pre = (double)tp/(double)(tp+fp);
	double rec = (double)tp/(double)(tp+fn);
	return (pre+rec)>0? 0 : (double)(2*pre*rec)/(double)(rec+pre);
	#endif
	//return ClassificationLearningEnvironment::getScore();
}

bool CIFAR::isTerminal() const
{
	return false;
}

uint8_t CIFAR::getCurrentImageLabel()
{
	return (uint8_t)this->currentClass;
}

void CIFAR::printClassifStatsTable(const Environment& env, const TPG::TPGVertex* bestRoot) {
	// Print table of classif of the best
	TPG::TPGExecutionEngine tee(env, NULL);

	// Change the MODE of cifar
	this->reset(0, Learn::LearningMode::TESTING);

	// Fill the table
	uint64_t classifTable[2][2] = { 0 };
	uint64_t nbPerClass[2] = { 0 };

	const int TOTAL_NB_IMAGE = 2000;
	for (int nbImage = 0; nbImage < TOTAL_NB_IMAGE; nbImage++) {
		// Get answer
		int currentLabel = this->getCurrentImageLabel();
		nbPerClass[currentLabel]++;
		// Execute
		auto path = tee.executeFromRoot(*bestRoot);
		const TPG::TPGAction* action = (const TPG::TPGAction*)path.at(path.size() - 1);
		uint8_t actionID = (uint8_t)action->getActionID();

		// Increment table
		classifTable[currentLabel][actionID]++;

		// Do action (to trigger image update)
		this->doAction(action->getActionID());
	}

	// Print the table
	//printf("\t");
	/*for (int i = 0; i < 2; i++) {
		printf("%d\t", i);
	}
	printf("Nb\n");
	for (int i = 0; i < 2; i++) {
		printf("%d\t", i);
		for (int j = 0; j < 2; j++) {
			if (i == j) {
				printf("\033[0;32m");
			}
			printf("%2.1f\t", 100.0 * (double)classifTable[i][j] / (double)nbPerClass[i]);
			if (i == j) {
				printf("\033[0m");
			}
		}
		printf("%4" PRIu64 "\n", nbPerClass[i]);
	}*/

	std::cout << std::endl;

	//linear
	//printf("\t linear : %lf \t", (double)score/(double)2000);

	double tp = (double)classifTable[0][0];
	double tf = (double)classifTable[1][1];
	double fp = (double)classifTable[1][0];
	double fn = (double)classifTable[0][1];

    double sum = tp + tn + fp + fn;
    
    double P_agree = (double)(tp + tn)/(double)sum;
    
    double P_tp = (double)(tp+fp)/(double)sum * (double)(tp + fn)/(double)sum;
    double P_tn = (double)(tn+fp)/(double)sum * (double)(tn + fn)/(double)sum;
    
    double P_rand = P_tp + P_tn; 
    
    double kappa = (double)(P_agree - P_rand)/(double)(1-P_rand);
    printf("\t kappa : %lf \t", kappa);

    //MCC
	/*int num = tp*tn-fp*fn;
	double densquare = (tp+fp)*(tp+fn)*(tn+fp)*(tn+fn);
	if(densquare >= 0)
	{	
		double res = (double)num / sqrt(densquare);
		printf("MCC: %lf \t", res);
	}
	else
	{
		printf("MCC: 0.000 \t");
	}

	//GSCORE
	double sen = (double)tp/(double)(tp+fn);
	double spe = (double)tn/(double)(tn+fp);
	printf("\t gscore : %lf \t", sqrt(sen * spe));

	//f1 score
	double pre = (double)tp/(double)(tp+fp);
	double rec = (double)tp/(double)(tp+fn);
	printf("\t f1score : %lf \t", (pre+rec)>0? 0 : (double)(2*pre*rec)/(double)(rec+pre));
	
	//return (double)this->score/100; // Basic TGP reward and linear compensation
	//return ClassificationLearningEnvironment::getScore();
    */
   	printf("%f\t", 100.0 * (double)classifTable[0][0] / (double)nbPerClass[0]);
	printf("%f\n", 100.0 * (double)classifTable[1][1] / (double)nbPerClass[1]);
    std::cout << std::endl;    
}

void CIFAR::processDataset()
{
		auto trainingSet = this->tmp_dataset.training_images;
		auto testSet = this->tmp_dataset.test_images;

		int count_minority_class = 0;
		int count_majority_class = 0;

		int index_tab[19] = {1,2,3,4,5,6,7,8,9,10,12,15,18,30,100,300,1000,3000,10000};
		/*double nb_sample_min[11]={1000, 1000, 900, 300, 90, 30, 9, 3, 0.9, 0.3, 0.09};
		int nb_sample_max[11]={1000, 3000, 9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000};
		*/

		double nb_sample_min[19]={1000, 666,  500,  400,  333,  285,  250,  222,  200,  166,  133,  111,   66,   20,    6,    2, 0.67, 0.2};
		double nb_sample_max[19]=   {1000,1344, 1500, 1600, 1667, 1715, 1750, 1778, 1800, 1844, 1867, 1889, 1934, 1980, 1994, 1999.33, 1999.8};

		int index = 0;

		for(int i = 0; i< 19; i++)
		{
			if(RATIO_IMB == index_tab[i])
			{
				index = i;
				break;
			}	
		}

		for(int i = 0; i < trainingSet.size(); ++i)
		{
			auto image = trainingSet.at(i);
			int classidx = this->tmp_dataset.training_labels.at(i);
			//downsample minority class
			if(classidx == TARGET_CLASS && count_minority_class >= 5*nb_sample_min[index])
			{
				continue;
			}

			//downsample majority class
			if(classidx != TARGET_CLASS && count_majority_class >= 5*nb_sample_max[index])
			{
				continue;
			}

			if(classidx == TARGET_CLASS)
			{
				count_minority_class++;
			}
			else
			{
				count_majority_class++;		
			}
			std::vector<double> tmpimage;
			for(int j = 0; j < 32; ++j)
			{
				for(int k = 0; k < 32; ++k)
				{
					int pxlIndex = 32*j+k;
					int32_t value = trainingSet.at(i).at(pxlIndex)<<16 | 
					trainingSet.at(i).at(32*32+pxlIndex)<<8 | 
					trainingSet.at(i).at(32*32*2+pxlIndex);
					tmpimage.push_back(value);
				}
			}
			this->dataset.training_images.push_back(tmpimage);
			if(classidx == TARGET_CLASS)
			{
				this->dataset.training_labels.push_back(0);
			}	
			else
			{
				this->dataset.training_labels.push_back(1);
			}
			
		}

		count_minority_class = 0;
		count_majority_class = 0;

		for(int i = 0; i < testSet.size(); ++i)
		{
			auto image = testSet.at(i);
			int classidx = (int)this->tmp_dataset.test_labels.at(i);
			//downsample minority class
			if(classidx == TARGET_CLASS && count_minority_class >= nb_sample_min[index])
			{
				continue;
			}

			//downsample majority class
			if(classidx != TARGET_CLASS && count_majority_class >= nb_sample_max[index])
			{
				continue;
			}

			if(classidx == TARGET_CLASS)
			{
				count_minority_class++;
			}
			else
			{
				count_majority_class++;		
			}
			
			std::vector<double> tmpimage;
			for(int j = 0; j < 32; ++j)
			{
				for(int k = 0; k < 32; ++k)
				{
					int pxlIndex = 32*j+k;
					int64_t value = testSet.at(i).at(pxlIndex)<<16 | 
					testSet.at(i).at(32*32+pxlIndex)<<8 | 
					testSet.at(i).at(32*32*2+pxlIndex);

					tmpimage.push_back(value);
				}
			}
			this->dataset.test_images.push_back(tmpimage);
			if(classidx == TARGET_CLASS)
			{
				this->dataset.test_labels.push_back(0);
			}	
			else
			{
				this->dataset.test_labels.push_back(1);
			}
		}

		this->tmp_dataset.training_images.empty();
		this->tmp_dataset.training_labels.empty();
		this->tmp_dataset.test_images.empty();
		this->tmp_dataset.test_labels.empty();

		Learn::LearningParameters params;
		File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json", params);
		//init subset
		/*for(int i = 0; i < 10; ++i)
		{
			int count = 0;
			while (count < params.maxNbActionsPerEval/10)
			{
				//generate a random number 
				int32_t index = rng.getUnsignedInt64(0, this->dataset.training_images.size() - 1);
				//we want the same number of samples of the same class in the dataset
				if(this->dataset.training_labels.at(index) == i)
				{
					this->subset.training_images.push_back(this->dataset.training_images.at(index));
					this->subset.training_labels.push_back(this->dataset.training_labels.at(index));
					++count;
				}
				
			}
		}*/
}