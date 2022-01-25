#include <random>
#include <inttypes.h>

#include "mnist.h"

mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> MNIST::dataset(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION));
mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> MNIST::subset;

void MNIST::changeCurrentImage()
{
	// Get the container for the current mode.
	std::vector<std::vector<double>>& dataSource = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->subset.training_images : this->subset.test_images;

	// Select the image 
	// If the mode is training or validation
	if (this->currentMode == Learn::LearningMode::TRAINING || this->currentMode == Learn::LearningMode::VALIDATION) {
		// Select a random image index
		this->currentIndex = rng.getUnsignedInt64(0, dataSource.size() - 1);
	}
	else {
		// If the mode is TESTING, just go to the next image
		this->currentIndex = (this->currentIndex + 1) % dataSource.size();
	}

	// Load the image in the dataSource
	for (uint64_t pxlIndex = 0; pxlIndex < 28 * 28; pxlIndex++) {
		this->currentImage.setPointer(&dataSource.at(this->currentIndex));
	}

	// Keep current label too.
	this->currentClass = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->subset.training_labels.at(this->currentIndex) : this->subset.test_labels.at(this->currentIndex);

}

MNIST::MNIST() : LearningEnvironment(10), currentImage(28, 28)
{
    computeSubset();
	// Fill shared dataset dataset(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION))
	/*if (MNIST::subset.training_labels.size() != 0) {
		std::cout << "Nbr of training images = " << subset.training_images.size() << std::endl;
		std::cout << "Nbr of training labels = " << subset.training_labels.size() << std::endl;
		std::cout << "Nbr of test images = " << subset.test_images.size() << std::endl;
		std::cout << "Nbr of test labels = " << subset.test_labels.size() << std::endl;
	}
	else {
		throw std::runtime_error("Initialization of MNIST databased failed.");
	}*/
}

void MNIST::doAction(uint64_t actionID)
{
    static int r = (dataset.training_images.size()-50*RATIO_IMB)/50*RATIO_IMB;
    // Call to default method to increment classificationTable
    if(this->currentClass == TARGET_CLASS && actionID == TARGET_CLASS) //TP
    {
        this->score += 1;  // basic TPG reward
        //this->score += r;  // linear compensation
        this->tp += 1;       // GScore, MCC or
    }
    else if(this->currentClass != TARGET_CLASS && actionID == this->currentClass) //TN
    {
        this->score += 1;
        this->tn += 1;
    }
    else if (this->currentClass != TARGET_CLASS && actionID == TARGET_CLASS) //FP
    {
        this->score -= 1;   //basic TPG reward
        //this->score -= r;   //linear compensation
        this->fp += 1;
    }
    else if (this->currentClass == TARGET_CLASS && actionID != TARGET_CLASS) //FN
    {
        this->score -= 1;
        this->fn += 1;
    }

	// Call to devault method to increment classificationTable
	//ClassificationLearningEnvironment::doAction(actionID);
    LearningEnvironment::doAction(actionID);

	this->changeCurrentImage();
}

void MNIST::reset(size_t seed, Learn::LearningMode mode)
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

std::vector<std::reference_wrapper<const Data::DataHandler>> MNIST::getDataSources()
{
	std::vector<std::reference_wrapper<const Data::DataHandler>> res = { currentImage };

	return res;
}

bool MNIST::isCopyable() const
{
	return true;
}

Learn::LearningEnvironment* MNIST::clone() const
{
	return new MNIST(*this);
}

double MNIST::getScore() const
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
    return (double)this->score/5000.0; // Basic TGP reward and linear compensation
#endif

    //F1
#if FITNESS==4
    //f1 score
	double pre = (double)tp/(double)(tp+fp);
	double rec = (double)tp/(double)(tp+fn);
	return (pre+rec)>0? 0 : (double)(2*pre*rec)/(double)(rec+pre);
#endif
    //return ClassificationLearningEnvironment::getScore();
	// Return the default classification score
	//return ClassificationLearningEnvironment::getScore();
}

bool MNIST::isTerminal() const
{
	return false;
}

uint8_t MNIST::getCurrentImageLabel()
{
	return (uint8_t)this->currentClass;
}

void MNIST::printClassifStatsTable(const Environment& env, const TPG::TPGVertex* bestRoot) {
	// Print table of classif of the best
	TPG::TPGExecutionEngine tee(env, NULL);

	// Change the MODE of mnist
	this->reset(0, Learn::LearningMode::TESTING);

	// Fill the table
	uint64_t classifTable[10][10] = { 0 };
	uint64_t nbPerClass[10] = { 0 };

	const int TOTAL_NB_IMAGE = 10000;
	for (int nbImage = 0; nbImage < TOTAL_NB_IMAGE; nbImage++) {
		// Get answer
		uint8_t currentLabel = this->getCurrentImageLabel();
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
	/*printf("\t");
	for (int i = 0; i < 10; i++) {
		printf("%d\t", i);
	}
	printf("Nb\n");
	for (int i = 0; i < 10; i++) {
		printf("%d\t", i);
		for (int j = 0; j < 10; j++) {
			if (i == j) {
				printf("\033[0;32m");
			}
			printf("%2.1f\t", 100.0 * (double)classifTable[i][j] / (double)nbPerClass[i]);
			if (i == j) {
				printf("\033[0m");
			}
		}
		printf("%4" PRIu64 "\n", nbPerClass[i]);
	}
	std::cout << std::endl;*/
	printf("\n%d \t %d \t %d \t %d \n", tp, tn, fp, fn);
}

double MNIST::computeKappa() {
    static double kappa = 0;
    double sum = tp + tn + fp + fn;

    double P_agree = (double)(tp + tn)/(double)sum;

    double P_tp = (double)(tp+fp)/(double)sum * (double)(tp + fn)/(double)sum;
    double P_tn = (double)(tn+fp)/(double)sum * (double)(tn + fn)/(double)sum;

    double P_rand = P_tp + P_tn;

    double tmp = (double)(P_agree - P_rand)/(double)(1-P_rand);
    if(kappa < tmp)
    {
        kappa = tmp;
        printf("\t kappa : %lf \t \n", kappa);
    }
    return kappa;
}

void MNIST::computeSubset() {
    int count_min_class = 0;
    int count_maj_class = 0;
    //                             1    2     3     4     5     6     7    8     9     10
    //double nb_sample_min[19]={1000, 666,  500,  400,  333,  285,  250,  222,  200,  166,  133,  111,   66,   20,    6,    2, 0.67, 0.2};
    double nb_sample_min[19]={1000, 500, 333,  166, 66, 20,    6,    2, 0.67, 0.2};
    //double nb_sample_max[19]={1000,1344, 1500, 1600, 1667, 1715, 1750, 1778, 1800, 1844, 1867, 1889, 1934, 1980, 1994, 1999.33, 1999.8};
    double nb_sample_max[19]={1000, 1500, 1667, 1844, 1934, 1980, 1994, 1999.33, 1999.8};
    /// MNIST dataset for the training.
    for(int i = 0;i < dataset.training_images.size(); i++)
    {
        if(dataset.training_labels.at(i) == TARGET_CLASS && count_min_class < 5*nb_sample_min[RATIO_IMB])
        {
            count_min_class++;
            subset.training_images.push_back(dataset.training_images.at(i));
            subset.training_labels.push_back(TARGET_CLASS);
        }
        else if (count_maj_class < 5*nb_sample_max[RATIO_IMB])
        {
            count_maj_class++;
            subset.training_images.push_back(dataset.training_images.at(i));
            subset.training_labels.push_back(dataset.training_labels.at(i));
        }
    }

    count_min_class = 0;
    count_maj_class = 0;

    for(int i = 0;i < dataset.test_images.size(); i++)
    {
        if(dataset.test_labels.at(i) == TARGET_CLASS && count_min_class < nb_sample_min[RATIO_IMB])
        {
            count_min_class++;
            subset.test_images.push_back(dataset.test_images.at(i));
            subset.test_labels.push_back(TARGET_CLASS);
        }
        else if (count_maj_class < nb_sample_max[RATIO_IMB])
        {
            count_maj_class++;
            subset.test_images.push_back(dataset.test_images.at(i));
            subset.test_labels.push_back(dataset.test_labels.at(i));
        }
    }
}

