#pragma once
#include <iostream> 
#include <map>
#include <optional>
#include <filesystem>


namespace journalUtils
{
    enum modType{
        addition = '+',
        removal = '-'
    };

    struct Transaction{
        std::string timeStamp;
        std::string lineContent;
        std::string lineNumber;
        char modType;

        Transaction(); 
        Transaction(std::string, std::string, std::string, char);

        friend std::ostream& operator<<(std::ostream&, const Transaction&); 
    };

    std::ostream& operator<<(std::ostream& os, const Transaction&);

    std::optional<Transaction> buildTransactionFromLines(std::string); 

    std::string getTimeStamp(); 

    std::filesystem::path getJournalPath(const std::filesystem::path&); 

    void createNewJournal(const std::filesystem::path&); 

    void createExistingFileJournals(const std::filesystem::path&);

    void initilizeDirectory(const std::filesystem::path&);

    void createJournalDirectory(const std::filesystem::path&);

    bool directoryHasJournal(const std::filesystem::path&);

    bool fileHasJournal(const std::filesystem::path&); 

    std::map<std::string, std::string> reconstructJournal(const std::filesystem::path&); 

    std::map<std::string, std::string> reconstructJournalFromSelectedDate(const std::filesystem::path&, const std::string&); 

    void updateJournal(const std::filesystem::path&);

    std::map<std::string, std::string> getReconstructionDates(const std::filesystem::path&); 

    
}
