

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include<iostream>
#include <string.h>

#include <fstream>
#include <vector>
#include "HybridAnomalyDetector.h"

using namespace std;

class DefaultIO{
public:
	virtual string read()=0;
	virtual void write(string text)=0;
	virtual void write(float f)=0;
	virtual void read(float* f)=0;
	virtual ~DefaultIO(){}

	// you may add additional methods here
	void createCSV(std::string fileName) {
		ofstream file(fileName);
		std::string line = read();
		while (line.compare("done") != 0) {
			file << line << std::endl;
			line = read();
		}
		write("Upload complete.\n");
	}
};

class StandardIO : public DefaultIO {
public:
	virtual string read() {
		std::string input;
		std::cin >> input;
		return input;
	}
	virtual void write(string text) {
		std::cout << text;
	}
	virtual void write(float f) {
		std::cout << f;
	}
	virtual void read(float* f) {
		std::cin >> *f;
	}
};

// you may add here helper classes
class Report {
public:
	string description;
	long timeStep;
	Report(string description, long timeStep):description(description),timeStep(timeStep){}
};

class DetectorData {
private:
	HybridAnomalyDetector ad = HybridAnomalyDetector();
	float correlation = ad.getThreshold();
public:
	vector<Report> reports;
	DetectorData() {}
	void anomalyTrain(const char* fileName) {
		TimeSeries tsTrain = TimeSeries(fileName);
		ad.learnNormal(tsTrain);
	}
	void anomalyTest(const char* fileName) {
		TimeSeries tsTest = TimeSeries(fileName);
		std::vector<AnomalyReport> reportsFromAd = ad.detect(tsTest);
		reports.clear();
		for (auto a : reportsFromAd) {
			reports.push_back(Report(a.description, a.timeStep));
		}
	}
	float getCorrelation() {return correlation;}
	void setCorrelation(float newCor) {
		ad.setThreshold(newCor);
		correlation = newCor;
	}
	~DetectorData(){
		reports.clear();
	}
};




// you may edit this class
class Command{
protected:
	DefaultIO* dio;
	DetectorData* dd;
public:
	Command(DefaultIO* dio, DetectorData* dd):dio(dio),dd(dd) {}
	virtual void execute()=0;
	virtual ~Command(){}
	virtual void print_description() {
		this->dio->write(description());
	}
	virtual std::string description()=0;
};

// implement here your command classes

class UploadCommand : public Command {
public:
	UploadCommand(DefaultIO* dio, DetectorData* dd) : Command(dio,dd) {}
	virtual std::string description() {return "1.upload a time series csv file\n";}
	virtual void execute() {
		std::string fileNames[] = {"anomalyTrain.csv", "anomalyTest.csv"};
		std::string instruction[] = {"Please upload your local train CSV file.\n",
			"Please upload your local test CSV file.\n"};
		std::string line;
		for (int i = 0; i <= 1; ++i) {
			dio->write(instruction[i]);
			dio->createCSV(fileNames[i]);
		}
	}
};

class AlgSettingsCommand : public Command {
public:
	AlgSettingsCommand(DefaultIO* dio, DetectorData* dd) : Command(dio,dd) {}
	virtual std::string description() {return "2.algorithm settings\n";}
	virtual void execute() {
		float newCor;
		float currentCor = this->dd->getCorrelation();
		dio->write("The current correlation threshold is ");
		dio->write(currentCor);
		dio->write("\n");
		dio->read(&newCor);
		while (newCor < 0 || newCor > 1) {
			dio->write("please choose a value between 0 and 1.\n");
			dio->read(&newCor);
		}
		this->dd->setCorrelation(newCor);
	}
};

class DetectCommand : public Command {
public:
	DetectCommand(DefaultIO* dio, DetectorData* dd) : Command(dio,dd) {}
	virtual std::string description() {return "3.detect anomalies\n";}
	virtual void execute() {
		dd->anomalyTrain("anomalyTrain.csv");
		dd->anomalyTest("anomalyTest.csv");
		dio->write("anomaly detection complete.\n");
	}
};


class DisplayResultsCommand : public Command {
public:
	DisplayResultsCommand(DefaultIO* dio, DetectorData* dd) : Command(dio,dd) {}
	virtual std::string description() {return "4.display results\n";}
	virtual void execute() {
		for (Report r : dd->reports) {
			dio->write(to_string(r.timeStep) + "\t" + r.description + "\n");
		}
		dio->write("Done.\n");
	}
};

class UploadAndAnalyzeCommand : public Command {
public:
	UploadAndAnalyzeCommand(DefaultIO* dio, DetectorData* dd) : Command(dio,dd) {}
	virtual std::string description() {return "5.upload anomalies and analyze results\n";}
	virtual void execute() {
	}
};

class ExitCommand : public Command {
public:
	ExitCommand(DefaultIO* dio, DetectorData* dd) : Command(dio,dd) {}
	virtual std::string description() {return "6.exit\n";}
	virtual void execute() {
	}
};


#endif /* COMMANDS_H_ */
