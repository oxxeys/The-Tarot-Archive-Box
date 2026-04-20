const express = require("express");
const path = require("path");
const app = express();


app.use(express.json());

app.post("/data", (req, res) => {
  console.log(req.body);
  res.send("OK");
});
 


const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`Server running on ${PORT}`));
