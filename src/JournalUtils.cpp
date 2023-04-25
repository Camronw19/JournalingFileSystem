#include "JournalUtils.h"
#include <inotify-cpp/NotifierBuilder.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <vector>


namespace journalUtils
{
    Transaction::Transaction() : timeStamp(""), lineContent(""), lineNumber("0"), modType('0'){}; 
    Transaction::Transaction(std::string tStamp, std::string lContent, std::string lNum, char mType) : timeStamp(tStamp), lineContent(lContent), lineNumber(lNum), modType(mType) {}; 
 
    std::ostream& operator<<(std::ostream& os, const Transaction& t)
    {
        os << t.timeStamp << "," << t.modType << "," << t.lineNumber << "," << t.lineContent << ","; 
        return os; 
    }

    std::optional<Transaction> buildTransactionFromLine(std::string line)
    {
        std::vector<std::string> fields; 
        std::string field; 

        for (char c : line)
        {
            if (c == ',')
            {
                fields.push_back(field); 
                field.clear(); 
            }
            else
            {
                field.push_back(c); 
            }
        }


        if (fields.size() == 4)
        {
            Transaction t;  
            t.timeStamp = fields[0]; 
            t.modType = fields[1][0]; 
            t.lineNumber = fields[2]; 
            t.lineContent = fields[3]; 
            return t; 
        }
        else 
        {
            return {}; 
        }

    }

    std::string getTimeStamp()
    {
        auto now = std::chrono::system_clock::now(); 
        std::time_t current_time = std::chrono::system_clock::to_time_t(now); 
        std::tm* local_time = std::localtime(&current_time); 
        
        std::ostringstream oss; 
        oss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S"); 
        std::string timeStamp = oss.str(); 
        return timeStamp;  
    }

    std::filesystem::path getJournalPath(const std::filesystem::path& file)
    {
        std::string fileName = file.filename().string(); 
        std::string journalName = fileName + "_journal.DAT"; 
        std::filesystem::path journalPath = file.parent_path() / ".journals" / journalName; 
        return journalPath; 
    }

    void createNewJournal(const std::filesystem::path& file)
    {
            std::cout  << "Creating new journal" << std::endl; 

            std::string fileName = file.filename().string(); 
            std::string journalName = fileName + "_journal.DAT"; 
            std::cout << "Journal name " + journalName << std::endl; 


            std::string journalDirectory = ".journals"; 
            std::filesystem::path journalDirPath = file.parent_path() / journalDirectory; 

            std::filesystem::path newFile = journalDirPath / journalName; 

            std::ofstream outfile(newFile); 
            if (outfile.is_open())
            {
                std::cout << "file created successfully" << std::endl; 
            }
            outfile.close(); 
    }

    void initilizeDirectory(const std::filesystem::path& dirPath)
    {
        if (!directoryHasJournal(dirPath))
        {
            createJournalDirectory(dirPath); 
            createExistingFileJournals(dirPath); 
        }
    }

    void createExistingFileJournals(const std::filesystem::path& dirPath)
    {
        if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) 
        {
            for (const auto& entry : std::filesystem::directory_iterator(dirPath)) 
            {
                if (std::filesystem::is_regular_file(entry) && !fileHasJournal(entry))
                {
                    createNewJournal(entry); 
                    updateJournal(entry); 
                }
            }
        } 
        else 
        {
            std::cerr << "The specified path is not a valid directory." << std::endl;
        }
    }

    bool fileHasJournal(const std::filesystem::path& filePath)
    {
        if(std::filesystem::exists(getJournalPath(filePath)))
        {
            return true; 
        } 
        else
        {
            return false; 
        }
    }


    void createJournalDirectory(const std::filesystem::path& watchedDirectory)
    {
        std::string journalDirectory = ".journals"; 
        std::filesystem::path dirPath = watchedDirectory / journalDirectory; 

        std::filesystem::create_directory(dirPath); 
        if (!std::filesystem::is_directory(dirPath))
        {
            std::cout << "Error: Failed to create directory" << std::endl; 
        }
    }

    bool directoryHasJournal(const std::filesystem::path& watchedDirectory)
    {
        std::string journalDirectory = ".journals"; 
        std::filesystem::path dirPath = watchedDirectory / journalDirectory; 

        for (const auto& dir : std::filesystem::directory_iterator(watchedDirectory))
        {
            if (dir.is_directory() && dir.path().filename() == journalDirectory)
            {
                std::cout << "journal dir already created" << std::endl; 
                return true; 
            }
        }
        std::cout << "creating journal dir" << std::endl; 
        return false; 
    }

    std::map<std::string, std::string> reconstructJournal(const std::filesystem::path& journalPath)
    {
        std::ifstream journal(journalPath);
        
        if(!journal) 
        {
            std::cout << "Failed to open journal: " << journalPath << std::endl; 
            return std::map<std::string, std::string>(); 
        }

        std::map<std::string, std::string> file;  
        std::map<std::string, std::string>::iterator it; 
        std::string journalLine; 

        while (!journal.eof())
        {
            getline(journal, journalLine); 
            std::optional<Transaction> t = buildTransactionFromLine(journalLine); 

            if (t.has_value())
            {
                if (t->modType == '+')
                {
                    file[t->lineNumber] = t->lineContent; 
                }
                else if (t->modType == '-')
                {
                    it = file.find(t->lineNumber); 
                    file.erase(it); 
                }
            }
        }
        return file; 
    }

    std::map<std::string, std::string> reconstructJournalFromSelectDate(const std::filesystem::path& filePath)
    {
        std::filesystem::path journalPath = getJournalPath(filePath); 
        std::ifstream journal(journalPath);
        
        if(!journal) 
        {
            std::cout << "Failed to open journal: " << journalPath << std::endl; 
            return std::map<std::string, std::string>(); 
        }

        std::string journalLine; 
        std::map<std::string, std::string> dates;  
        std::map<std::string, std::string>::iterator it; 
        
        // Get dates that the journal can be restored from
        while (getline(journal, journalLine))
        {
            std::optional<Transaction> t = buildTransactionFromLine(journalLine); 
            bool found = false;

            if (t.has_value())
            {
                for (const auto& date : dates)
                {
                    if (date.second == t->timeStamp)
                        found = true; 
                }

                if (!found) 
                {
                    dates[t->lineNumber] = t->timeStamp; 
                }
            }
        }

        journal.close(); 

        // Get user input for date to restore to 
        if (dates.empty())
            std::cout << "No previous versions to restore from" << std::endl; 
        else
        {        
            std::cout << "Select a date to restore the journal from: " << std::endl; 
            for (const auto& date : dates)
            {
                std::cout << date.second <<  std::endl; 
            }
        }

        //get date to restore from
        bool matchFound = false; 
        while (!matchFound)
        {
            std::string selectDate;

            std::getline(std::cin, selectDate); 

            for (it = dates.begin(); it != dates.end(); ++it) 
            {
                if (selectDate == it->second) 
                {
                    ++it; 
                    matchFound = true;
                    break;
                }
            }

            if (!matchFound)
            {
                std::cout << "Invalid entry, please enter a valid date" << std::endl; 
            }
                
        }
        

        //restore the file 
        std::string lineNumber = it->first; 
        std::map<std::string, std::string> rFile; 
        journal.open(journalPath);  

          if(!journal) 
        {
            std::cout << "Failed to open journal: " << journalPath << std::endl; 
            return std::map<std::string, std::string>(); 
        }

        while (getline(journal, journalLine))
        {
            std::optional<Transaction> t = buildTransactionFromLine(journalLine); 

            if (t.has_value() && t->lineNumber != lineNumber)
            {
                if (t->modType == '+')
                {
                    rFile[t->lineNumber] = t->lineContent; 
                }
                else if (t->modType == '-')
                {
                    it = rFile.find(t->lineNumber); 
                    rFile.erase(it); 
                }
            }
            else if (t->lineNumber == lineNumber)
            {
                break; 
            }
        }

        journal.close(); 

        //overrite file contents 

        std::ofstream oFile;
        oFile.open(filePath, std::ios::out);

        if (!oFile.is_open()) {
            std::cerr << "Failed to open the file!" << std::endl;
        }

        for (const auto& line : rFile)
        {
            oFile << line.second << std::endl; 
        }

        oFile.close();
        std::cout << "Journal restored succesfully" << std::endl; 

        //return the file as a vector of strings or map from line number to string
        return rFile; 
    } 

    void updateJournal(const std::filesystem::path& path)
    {
        std::vector<Transaction> transactions; 
        
        std::filesystem::path journalPath = getJournalPath(path);
        std::filesystem::path filePath = path; 

        //file and reconstructed journal for comparisons 
        std::ifstream file(filePath); 
        std::map<std::string, std::string> rJournal = reconstructJournal(journalPath); 
        std::map<std::string, std::string>::iterator it; 

        if(!file) 
        {
            std::cout << "Failed to open file: " << filePath << std::endl;
            return; 
        }

        std::cout << "Writing differences to journal" << std::endl; 
        std::string fileLine; 
        std::string journalLine; 

        int lineNum = 0; 
        std::string timeStamp = getTimeStamp(); 

        //compare file to reconstructed journal and write differences to the journal
        while (!file.eof())
        {   
            lineNum++; 
            getline(file, fileLine); 
            it = rJournal.find(std::to_string(lineNum)); 

            if (it == rJournal.end())
                journalLine = ""; 
            else
                journalLine = it->second; 

            if (fileLine != journalLine)
            {
                if (journalLine.empty() and !fileLine.empty())
                {
                    transactions.emplace_back(Transaction(timeStamp, fileLine, std::to_string(lineNum), addition)); 
                }
                else
                {
                    transactions.emplace_back(Transaction(timeStamp, journalLine, std::to_string(lineNum), removal)); 
                    transactions.emplace_back(Transaction(timeStamp, fileLine, std::to_string(lineNum), addition)); 
                }
            }
        }

        file.close(); 

        //write files to journal
        std::ofstream journal(journalPath, std::ios::app); 

        if (!journal.is_open())
        {
            std::cout << "Unable to open journal for writing" <<  std::endl;
        }    
        else
        {
            for (auto t : transactions)
            {
                journal << t << std::endl;  
            }
        }
    }
}
