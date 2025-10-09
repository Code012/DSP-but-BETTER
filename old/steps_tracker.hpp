#include <iostream>

struct SimplificationStep {
    std::string rule;                       // e.g. "Distributed Rule"
    std::string description;                // Human-readable explanation
    std::string before;                     // Full expression before applying rule
    std::string after;                      // Full expression after applying rule
    std::map<int, std::string> highlights;  // Node ids mapped to highlight part of expression in UI
};


    


// class TransformationTracker {
//     public:
//         struct Step {
//             int recursionDepth;         // Tracks nesting level
//             std::string nodePath;       // Path to node (e.g., "root.left.right")
//             std::string ruleName;
//             std::string description;
//             std::string beforeState;
//             std::string afterState;
//             std::string explanation;
//         };
        
//         // // Start a new recursion level (when entering a node)
//         // void enterNode(const std::string& nodeType, const std::string& nodeStr) {
//         //     currentPath.push_back(nodeType);
//         //     depthStack.push_back(nodeStr);
//         //     currentDepth++;
//         // }
        
//         // // End a recursion level (when leaving a node)
//         // std::string exitNode() {
//         //     std::string result = depthStack.back();
//         //     depthStack.pop_back();
//         //     if (!currentPath.empty()) {
//         //         currentPath.pop_back();
//         //     }
//         //     currentDepth--;
//         //     return result;
//         // }
        
//         // Record a transformation step
//         void recordStep(
//             const std::string& ruleName,
//             const std::string& description,
//             const std::string& beforeState,
//             const std::string& afterState) {
            
//             std::string pathStr = buildPathString();
            
//             steps.push_back({
//                 currentDepth,
//                 pathStr,
//                 ruleName,
//                 description,
//                 beforeState,
//                 afterState,
//                 ""  // Explanation will be generated later
//             });
//         }
        
//         // Get all recorded steps
//         const std::vector<Step>& getSteps() const {
//             return steps;
//         }
        
//         // Generate readable explanations for all steps
//         void generateExplanations() {
//             // First pass: mark steps that should be merged
//             std::vector<bool> shouldMerge(steps.size(), false);
            
//             // Simple rules for merging: consecutive steps on same node with same rule
//             for (size_t i = 1; i < steps.size(); i++) {
//                 if (steps[i].nodePath == steps[i-1].nodePath && 
//                     steps[i].ruleName == steps[i-1].ruleName) {
                    
//                     shouldMerge[i] = true;
//                 }
//             }
            
//             // Second pass: generate explanations (with context awareness)
//             for (size_t i = 0; i < steps.size(); i++) {
//                 if (!shouldMerge[i]) {
//                     steps[i].explanation = generateExplanation(steps[i], i);
//                 }
//             }
            
//             // Third pass: filter out merged steps
//             std::vector<Step> filteredSteps;
//             for (size_t i = 0; i < steps.size(); i++) {
//                 if (!shouldMerge[i]) {
//                     filteredSteps.push_back(steps[i]);
//                 }
//             }
            
//             steps = filteredSteps;
//         }
        
//     private:
//         std::vector<Step> steps;
//         std::vector<std::string> currentPath;     // Path to current node
//         std::vector<std::string> depthStack;      // Expression at each depth
//         int currentDepth = 0;                     // Current recursion depth
        
//         // Build a string representing the current path
//         std::string buildPathString() const {
//             std::stringstream ss;
//             for (size_t i = 0; i < currentPath.size(); i++) {
//                 if (i > 0) ss << ".";
//                 ss << currentPath[i];
//             }
//             return ss.str();
//         }
        
//         // Generate human-readable explanation
//         std::string generateExplanation(const Step& step, size_t stepIndex) {
//             // Look at rule name to generate appropriate explanation
//             if (step.ruleName == "SPRD-1") {
//                 return "I found an undefined term in the multiplication, so the entire result is undefined.";
//             }
//             else if (step.ruleName == "SPRD-2") {
//                 return "I noticed there's a zero in this multiplication. Any number multiplied by zero equals zero.";
//             }
//             else if (step.ruleName == "SPRDREC-1-1") {
//                 return "I multiplied the constants " + step.beforeState + " to get " + step.afterState;
//             }
//             else if (step.ruleName == "SPRDREC-1-2") {
//                 return "I removed the factor of 1 since multiplying any number by 1 doesn't change it.";
//             }
//             else if (step.ruleName == "distribution") {
//                 return "I used the distributive property to multiply " + step.beforeState + 
//                        ". This means I multiply each term inside the parentheses by the factor outside.";
//             }
//             // Add more rule-specific explanations...
            
//             return "I simplified " + step.beforeState + " to get " + step.afterState;
//         }
//     };