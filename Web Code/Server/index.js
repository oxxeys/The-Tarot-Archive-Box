const express = require("express");
const path = require("path");
const app = express();

app.use(express.json());


const mongoose = require("mongoose");
mongoose.connect("mongodb+srv://kiran1104_db_user:VsxQ2SCnphgCEGVY@floridium.vkjfn7c.mongodb.net/floridium?retryWrites=true&w=majority")
  .then(() => {
    console.log("Connected to the DB");
  })
  .catch(err => {
    console.log("Can't find the database!", err)
    process.exit();
  })

const plantDataSchema = new mongoose.Schema({
  box_id: String,
  moisture_reading: String,
  temprature_reading: String,
  sunlight_reading: String,
  reading_time: { type: Date, default: Date.now }
})

const data = mongoose.model("data", plantDataSchema);

app.post("/data", async (req, res) => {
  
  try{
    console.log(req.body);
    const newData = new data({
      box_id: req.body.box_id,
      moisture_reading: req.body["Soil Moisture"],
      temprature_reading: req.body.Temprature,
      sunlight_reading: req.body.Sunlight
    });

    await newData.save();
    
    res.send("Saved To DB");

  }
  catch (err){
    console.error(err);
    res.status(500).send("Error saving data");
  }
  


  
});


const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`Server running on ${PORT}`));
