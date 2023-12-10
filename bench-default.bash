
hyperfine --runs 2 "./main < ./res5000/in" --shell=sh --export-json "./res5000/default.json"
hyperfine --runs 2 "./main < ./res3k100i400/in" --shell=sh --export-json "./res3k100i400/default.json"
hyperfine --runs 2 "./main < ./res3k1000i400/in" --shell=sh --export-json "./res3k1000i400/default.json"
hyperfine --runs 2 "./main < ./res3k5000i400/in" --shell=sh --export-json "./res3k5000i400/default.json"



