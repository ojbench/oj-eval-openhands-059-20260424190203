
#include<iostream>
#include<cstdio>
#include<cstring>
#include<cmath>
#include<string>

class BaseJudger {
public:
    BaseJudger(size_t time_limit, size_t memory_limit, const char *answer) 
        : time_limit_(time_limit), memory_limit_(memory_limit), score_(0) {
        if (answer != nullptr) {
            answer_ = new char[strlen(answer) + 1];
            strcpy(answer_, answer);
        } else {
            answer_ = nullptr;
        }
    }

    virtual void Submit(size_t time, size_t memory, const char *output) = 0;

    size_t GetScore() const { return score_; }

    virtual ~BaseJudger() {
        if (answer_ != nullptr) {
            delete[] answer_;
            answer_ = nullptr;
        }
    };

protected:
    char *answer_;
    const size_t time_limit_;
    const size_t memory_limit_;
    size_t score_;

    virtual bool CheckAnswer(const char *output) const {
        // the output must equal to the answer
        return strcmp(answer_, output) == 0;
    }
};

class OIJudger : public BaseJudger {
private:
    bool has_submitted_;

public:
    OIJudger(size_t time_limit, size_t memory_limit, const char *answer)
        : BaseJudger(time_limit, memory_limit, answer), has_submitted_(false) {}

    void Submit(size_t time, size_t memory, const char *output) override {
        // If already submitted, set score to 0
        if (has_submitted_) {
            score_ = 0;
            return;
        }
        
        // Mark as submitted
        has_submitted_ = true;
        
        // Check if within limits
        if (time > time_limit_ || memory > memory_limit_) {
            score_ = 0;
            return;
        }
        
        // Check answer with trailing spaces ignored
        if (CheckAnswer(output)) {
            score_ = 100;
        } else {
            score_ = 0;
        }
    }

protected:
    bool CheckAnswer(const char *output) const override {
        if (output == nullptr || answer_ == nullptr) {
            return output == answer_;
        }
        
        std::string output_str(output);
        std::string answer_str(answer_);
        
        // Remove trailing spaces from each line
        auto remove_trailing_spaces = [](std::string& str) {
            // Find the last non-space character
            size_t end_pos = str.find_last_not_of(" \t");
            if (end_pos != std::string::npos) {
                str = str.substr(0, end_pos + 1);
            } else {
                // String contains only spaces
                str.clear();
            }
        };
        
        // Split strings by lines and compare each line
        size_t output_pos = 0, answer_pos = 0;
        while (output_pos < output_str.length() || answer_pos < answer_str.length()) {
            // Find next line break or end of string
            size_t output_line_end = output_str.find('\n', output_pos);
            if (output_line_end == std::string::npos) output_line_end = output_str.length();
            
            size_t answer_line_end = answer_str.find('\n', answer_pos);
            if (answer_line_end == std::string::npos) answer_line_end = answer_str.length();
            
            // Extract lines
            std::string output_line = output_str.substr(output_pos, output_line_end - output_pos);
            std::string answer_line = answer_str.substr(answer_pos, answer_line_end - answer_pos);
            
            // Remove trailing spaces
            remove_trailing_spaces(output_line);
            remove_trailing_spaces(answer_line);
            
            // Compare lines
            if (output_line != answer_line) {
                return false;
            }
            
            // Move to next line
            output_pos = (output_line_end < output_str.length()) ? output_line_end + 1 : output_str.length();
            answer_pos = (answer_line_end < answer_str.length()) ? answer_line_end + 1 : answer_str.length();
        }
        
        return true;
    }
};

class ICPCJudger : public BaseJudger {
private:
    size_t best_score_;

public:
    ICPCJudger(size_t time_limit, size_t memory_limit, const char *answer)
        : BaseJudger(time_limit, memory_limit, answer), best_score_(0) {}

    void Submit(size_t time, size_t memory, const char *output) override {
        size_t current_score = 0;
        
        // Check if within limits
        if (time <= time_limit_ && memory <= memory_limit_) {
            // Check answer
            if (CheckAnswer(output)) {
                current_score = 100;
            }
        }
        
        // Update best score
        if (current_score > best_score_) {
            best_score_ = current_score;
        }
        
        // Update final score
        score_ = best_score_;
    }
};

class SpacialJudger : public BaseJudger {
private:
    const size_t full_score_time_;
    const size_t full_score_memory_;
    size_t best_score_;

    // Calculate score based on time
    size_t CalculateTimeScore(size_t time) const {
        if (time <= full_score_time_) {
            return 100;
        } else if (time >= time_limit_) {
            return 0;
        } else {
            // Linear interpolation between full_score_time and time_limit
            double ratio = (double)(time_limit_ - time) / (time_limit_ - full_score_time_);
            return (size_t)floor(ratio * 100);
        }
    }

    // Calculate score based on memory
    size_t CalculateMemoryScore(size_t memory) const {
        if (memory <= full_score_memory_) {
            return 100;
        } else if (memory >= memory_limit_) {
            return 0;
        } else {
            // Linear interpolation between full_score_memory and memory_limit
            double ratio = (double)(memory_limit_ - memory) / (memory_limit_ - full_score_memory_);
            return (size_t)floor(ratio * 100);
        }
    }

public:
    SpacialJudger(size_t time_limit, size_t memory_limit, size_t full_score_time, 
                  size_t full_score_memory, const char *answer)
        : BaseJudger(time_limit, memory_limit, answer), 
          full_score_time_(full_score_time), full_score_memory_(full_score_memory), 
          best_score_(0) {}

    void Submit(size_t time, size_t memory, const char *output) override {
        size_t current_score = 0;
        
        // Check if answer is correct
        if (CheckAnswer(output)) {
            // Calculate time and memory scores
            size_t time_score = CalculateTimeScore(time);
            size_t memory_score = CalculateMemoryScore(memory);
            
            // Final score is floor(time_score * memory_score / 100)
            current_score = (size_t)floor((double)(time_score * memory_score) / 100.0);
        }
        
        // Update best score
        if (current_score > best_score_) {
            best_score_ = current_score;
        }
        
        // Update final score
        score_ = best_score_;
    }
};
