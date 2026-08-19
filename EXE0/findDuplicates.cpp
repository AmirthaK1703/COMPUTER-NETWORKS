class Solution {
public:
    vector<int> findDuplicates(vector<int>& nums) {
        vector<int> res;
        int n=nums.size();
        vector<int> counts(n+1,0);
        for(int num:nums)
        {
            counts[num]++;
            if (counts[num]==2)
            {
                res.push_back(num);
            }
        }
       return res;
    }
};
