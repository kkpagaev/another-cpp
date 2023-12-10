// Get command-line arguments
const args = process.argv.slice(2);

// Check if there are any arguments
if (args.length !== 4) {
  console.log('Usage: node in_generator.js I m n k');
  process.exit(1);
} 

const I = parseInt(args[0]);
const m = parseInt(args[1]);
const n = parseInt(args[2]);
const k = parseInt(args[3]);

const threeplets = [];

for (let i = 0; i < k; i++) {
  const random_x = Math.floor(Math.random() * m);
  const random_y = Math.floor(Math.random() * n);
  const random_value = Math.random() * 100;
  threeplets.push([random_x, random_y, random_value]);
}

threeplets.sort((a, b) => {
  if (a[0] !== b[0]) {
    return a[0] - b[0];
  } else {
    return a[1] - b[1];
  }
})

console.log(I);
console.log(m + " " + n);
console.log(k);
for (const tree of threeplets) {
  console.log(tree.join(' '));
}

