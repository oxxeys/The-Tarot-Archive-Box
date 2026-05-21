const mongoose = require("mongoose");
const { Schema, model } = mongoose;

const dataSchema = new Schema({
  boxID: String,
  data: [Number],
});

const boxData = model("data", dataSchema);

//add box data to database
async function addNewBoxData(boxID, data) {
  let newBoxData = {
    boxID: boxID,
    data: data,
  };
  await boxData.create(newBoxData);
  return true;
}

async function getDataFromDB(boxID) {
  let recievedData = [];
  recievedData = await boxData.find({ boxID: boxID }).sort({ _id: -1 }).skip(1); // skip first one
  return recievedData;
}

async function getOneDataFromDB(boxID) {
  let recievedData = [];
  recievedData = await boxData.findOne({ boxID: boxID }).sort({ _id: -1 });
  return recievedData;
}

module.exports = {
  addNewBoxData,
  getDataFromDB,
  getOneDataFromDB,
};
